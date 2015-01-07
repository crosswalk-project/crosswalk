// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/types.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <libgen.h>

#include <glib.h>
#include <gio/gio.h>
#include <gio/gunixfdlist.h>

#include "xwalk/application/tools/linux/dbus_connection.h"
#include "xwalk/application/tools/linux/xwalk_extension_process_launcher.h"
#if defined(OS_TIZEN)
#include "url/gurl.h"
#include "xwalk/application/tools/linux/xwalk_launcher_tizen.h"
#include "xwalk/application/tools/tizen/xwalk_tizen_user.h"
#endif

static const char* xwalk_service_name = "org.crosswalkproject.Runtime1";
static const char* xwalk_running_path = "/running1";
static const char* xwalk_running_manager_iface =
    "org.crosswalkproject.Running.Manager1";
static const char* xwalk_running_app_iface =
    "org.crosswalkproject.Running.Application1";

static char* application_object_path;

static GMainLoop* mainloop;
static GDBusConnection* g_connection;
static GDBusObjectManager* g_running_apps_manager;
static XWalkExtensionProcessLauncher* ep_launcher = NULL;

static int g_argc;
static char** g_argv;
static gboolean query_running = FALSE;
static gboolean fullscreen = FALSE;
static gboolean remote_debugging = FALSE;
static gchar** cmd_appid_or_url;

static GOptionEntry entries[] = {
  { "running", 'r', 0, G_OPTION_ARG_NONE, &query_running,
    "Check whether the application is running", NULL },
  { "fullscreen", 'f', 0, G_OPTION_ARG_NONE, &fullscreen,
    "Run the application as fullscreen", NULL },
  { "debugging_port", 'd', 0, G_OPTION_ARG_NONE, &remote_debugging,
    "Enable remote debugging for the application", NULL },
  { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &cmd_appid_or_url,
    "ID of the application to be launched or URL to open", NULL },
  { NULL }
};

static void object_removed(GDBusObjectManager* manager, GDBusObject* object,
                           gpointer user_data) {
  const char* path = g_dbus_object_get_object_path(object);

  if (g_strcmp0(path, application_object_path))
    return;

  fprintf(stderr, "Application '%s' disappeared, exiting.\n", path);

  delete ep_launcher;
  g_main_loop_quit(mainloop);
}

static void on_app_properties_changed(GDBusProxy* proxy,
                                      GVariant* changed_properties,
                                      GStrv invalidated_properties,
                                      gpointer user_data) {
  const char* interface = g_dbus_proxy_get_interface_name(proxy);

  fprintf(stderr, "properties changed %s\n", interface);

  if (g_variant_n_children(changed_properties) == 0)
    return;

  if (g_strcmp0(interface, xwalk_running_app_iface))
    return;

  GVariantIter* iter;
  const gchar* key;
  GVariant* value;

  g_variant_get(changed_properties, "a{sv}", &iter);

  while (g_variant_iter_loop(iter, "{&sv}", &key, &value)) {
    if (g_strcmp0(key, "State"))
      continue;

    const gchar* state = g_variant_get_string(value, NULL);

    fprintf(stderr, "Application state %s\n", state);
  }
}

static gboolean init_extension_process_channel(GDBusProxy* app_proxy) {
  if (ep_launcher->is_started())
    return FALSE;

  // Get the client socket file descriptor from fd_list. The reply will
  // contains an index to the list.
  GUnixFDList* fd_list;
  GVariant* res = g_dbus_proxy_call_with_unix_fd_list_sync(
      app_proxy, "GetEPChannel", NULL, G_DBUS_CALL_FLAGS_NONE,
      -1, NULL, &fd_list, NULL, NULL);
  if (!res || g_variant_n_children(res) != 2)
    return FALSE;

  const gchar* channel_id =
      g_variant_get_string(g_variant_get_child_value(res, 0), NULL);
  if (!channel_id || !strlen(channel_id))
    return FALSE;

  gint32 client_fd_idx =
      g_variant_get_handle(g_variant_get_child_value(res, 1));
  int client_fd = g_unix_fd_list_get(fd_list, client_fd_idx, NULL);

  ep_launcher->Launch(channel_id, client_fd);
  return TRUE;
}

static void on_app_signal(GDBusProxy* proxy,
                          gchar* sender_name,
                          gchar* signal_name,
                          GVariant* parameters,
                          gpointer user_data) {
  if (!strcmp(signal_name, "EPChannelCreated")) {
    init_extension_process_channel(proxy);
  } else {
    fprintf(stderr, "Unkown signal received: %s\n", signal_name);
  }
}

static int query_application_running(const char* app_id) {
  GList* objects = g_dbus_object_manager_get_objects(g_running_apps_manager);
  GList* it;
  bool is_running = FALSE;

  for (it = objects; it; it = it->next) {
    GDBusObject* object = reinterpret_cast<GDBusObject*>(it->data);
    GDBusInterface* iface = g_dbus_object_get_interface(
        object,
        xwalk_running_app_iface);
    if (!iface)
      continue;

    GDBusProxy* proxy = G_DBUS_PROXY(iface);
    GVariant* id_variant;
    id_variant = g_dbus_proxy_get_cached_property(proxy, "AppID");
    if (!id_variant) {
      g_object_unref(iface);
      continue;
    }

    const gchar* id;
    g_variant_get(id_variant, "s", &id);
    if (!strcmp(app_id, id)) {
      is_running = TRUE;
      break;
    }

    g_object_unref(iface);
  }
  const char* str = is_running ? "running" : "not running";
  g_print("Application %s is %s.\n", app_id, str);

  g_list_free_full(objects, g_object_unref);
  return is_running ? 0 : 1;
}

static void launch_application(const char* appid_or_url,
                               gboolean fullscreen,
                               gboolean remote_debugging) {
  ep_launcher = new XWalkExtensionProcessLauncher();
  GError* error = NULL;
  g_signal_connect(g_running_apps_manager, "object-removed",
                   G_CALLBACK(object_removed), NULL);

  GDBusProxy* running_proxy = g_dbus_proxy_new_sync(
      g_connection,
      G_DBUS_PROXY_FLAGS_NONE, NULL, xwalk_service_name,
      xwalk_running_path, xwalk_running_manager_iface, NULL, &error);
  if (!running_proxy) {
    g_print("Couldn't create proxy for '%s': %s\n", xwalk_running_manager_iface,
            error->message);
    g_error_free(error);
    exit(1);
  }

  unsigned int launcher_pid = getpid();

  GVariant* result  = g_dbus_proxy_call_sync(running_proxy, "Launch",
      g_variant_new("(subb)", appid_or_url, launcher_pid, fullscreen,
                    remote_debugging),
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  if (!result) {
    fprintf(stderr, "Couldn't call 'Launch' method: %s\n", error->message);
    exit(1);
  }

  g_variant_get(result, "(o)", &application_object_path);
  fprintf(stderr, "Application launched with path '%s'\n",
          application_object_path);

  GDBusProxy* app_proxy = g_dbus_proxy_new_sync(
      g_connection,
      G_DBUS_PROXY_FLAGS_NONE, NULL, xwalk_service_name,
      application_object_path, xwalk_running_app_iface, NULL, &error);
  if (!app_proxy) {
    g_print("Couldn't create proxy for '%s': %s\n", xwalk_running_app_iface,
            error->message);
    g_error_free(error);
    exit(1);
  }

  g_signal_connect(app_proxy, "g-properties-changed",
                   G_CALLBACK(on_app_properties_changed), NULL);

  mainloop = g_main_loop_new(NULL, FALSE);
  g_signal_connect(app_proxy, "g-signal", G_CALLBACK(on_app_signal), NULL);

#if defined(OS_TIZEN)
  char name[128];
  snprintf(name, sizeof(name), "xwalk-%s", appid_or_url);

  if (xwalk_appcore_init(g_argc, g_argv, name, app_proxy)) {
    fprintf(stderr, "Failed to initialize appcore");
    exit(1);
  }
#endif

  init_extension_process_channel(app_proxy);
  g_main_loop_run(mainloop);
}

void connect_to_application_manager() {
  GError* error = NULL;
  g_connection = get_session_bus_connection(&error);
  if (!g_connection) {
    fprintf(stderr, "Couldn't get the session bus connection: %s\n",
            error->message);
    exit(1);
  }

  g_running_apps_manager =
      g_dbus_object_manager_client_new_sync(
          g_connection, G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
          xwalk_service_name, xwalk_running_path,
          NULL, NULL, NULL, NULL, &error);
  if (!g_running_apps_manager) {
    fprintf(stderr, "Service '%s' does could not be reached: %s\n",
            xwalk_service_name, error->message);
    exit(1);
  }
}

int main(int argc, char** argv) {
  GError* error = NULL;
  char* appid_or_url;

  g_argc = argc;
  g_argv = argv;

#if !GLIB_CHECK_VERSION(2, 36, 0)
  // g_type_init() is deprecated on GLib since 2.36.
  g_type_init();
#endif

#if defined(OS_TIZEN)
  if (xwalk_tizen_check_group_users())
    exit(1);
#endif

  GOptionContext* context =
      g_option_context_new("- Crosswalk Application Launcher");
  g_option_context_add_main_entries(context, entries, NULL);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    fprintf(stderr, "Option parsing failed: %s\n", error->message);
    exit(1);
  }

  connect_to_application_manager();

  // Launch app.
  if (!strcmp(basename(argv[0]), "xwalk-launcher")) {
    if (cmd_appid_or_url == NULL) {
      fprintf(stderr, "No AppID informed, nothing to do.\n");
      return 0;
    }
    appid_or_url = strdup(cmd_appid_or_url[0]);
#if defined(OS_TIZEN)
    if (GURL(appid_or_url).spec().empty()
       && xwalk_change_cmdline(argc, argv, appid_or_url))
      exit(1);
#endif
  } else {
    appid_or_url = strdup(basename(argv[0]));
  }


  // Query app.
  if (query_running) {
    return query_application_running(appid_or_url);
  }
#if defined(OS_TIZEN)
  if (xwalk_is_debugging_port_request_by_env())
    remote_debugging = TRUE;
#endif
  launch_application(appid_or_url, fullscreen, remote_debugging);
  free(appid_or_url);
  return 0;
}
