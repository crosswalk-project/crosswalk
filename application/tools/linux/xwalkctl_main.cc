// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <glib.h>
#include <gio/gio.h>
#include <locale.h>

#include <string>
#include <set>

#include "xwalk/application/tools/linux/dbus_connection.h"
#if defined(OS_TIZEN)
#include "xwalk/application/tools/linux/xwalk_tizen_user.h"
#endif

static const char* xwalk_service_name = "org.crosswalkproject.Runtime1";
static const char* xwalk_installed_path = "/installed1";
static const char* xwalk_installed_iface =
    "org.crosswalkproject.Installed.Manager1";
static const char* xwalk_installed_app_iface =
    "org.crosswalkproject.Installed.Application1";
static const char* xwalk_running_path = "/running1";
static const char* xwalk_running_manager_iface =
    "org.crosswalkproject.Running.Manager1";
static const char* xwalk_running_app_iface =
    "org.crosswalkproject.Running.Application1";

static char* install_path;
static char* uninstall_appid;
static gboolean list_with_status = FALSE;
static GDBusConnection* g_connection;

static GOptionEntry entries[] = {
  { "install", 'i', 0, G_OPTION_ARG_STRING, &install_path,
    "Path of the application to be installed/updated", "PATH" },
  { "uninstall", 'u', 0, G_OPTION_ARG_STRING, &uninstall_appid,
    "Uninstall the application with this appid", "APPID" },
  { "status", 's', 0, G_OPTION_ARG_NONE, &list_with_status,
    "List running applications", NULL },
  { NULL }
};

GDBusObjectManager* get_xwalk_object_manager(const char* object_path) {
  GError* error = NULL;
  GDBusObjectManager* object_manager = g_dbus_object_manager_client_new_sync(
      g_connection, G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
      xwalk_service_name, object_path,
      NULL, NULL, NULL, NULL, &error);
  if (!object_manager) {
    g_print("Service '%s' could not be reached: %s\n", xwalk_service_name,
            error->message);
    return NULL;
  }

  // GDBus may return a valid ObjectManager even if it fails to auto activate
  // the proper service name. We should check 'name-owner' property to make sure
  // the service is working. See GDBusObjectManager documentation.
  gchar* name_owner = NULL;
  g_object_get(object_manager, "name-owner", &name_owner, NULL);
  if (!name_owner) {
    g_print("Service '%s' could not be activated.\n", xwalk_service_name);
    return NULL;
  }
  g_free(name_owner);

  return object_manager;
}

static bool install_application(const char* path) {
  GError* error = NULL;
  GDBusProxy* proxy;
  bool ret;
  GVariant* result = NULL;

  proxy = g_dbus_proxy_new_sync(
      g_connection,
      G_DBUS_PROXY_FLAGS_NONE, NULL, xwalk_service_name,
      xwalk_installed_path, xwalk_installed_iface, NULL, &error);
  if (!proxy) {
    g_print("Couldn't create proxy for '%s': %s\n", xwalk_installed_iface,
            error->message);
    g_error_free(error);
    ret = false;
    goto done;
  }

  result = g_dbus_proxy_call_sync(proxy, "Install",
                                  g_variant_new("(s)", path),
                                  G_DBUS_CALL_FLAGS_NONE,
                                  -1, NULL, &error);
  if (!result) {
    g_print("Installing application failed: %s\n", error->message);
    g_error_free(error);
    ret = false;
    goto done;
  }

  const char* object_path;

  g_variant_get(result, "(o)", &object_path);
  g_print("Application installed/updated with path '%s'\n", object_path);
  g_variant_unref(result);

  ret = true;

 done:
  if (proxy)
    g_object_unref(proxy);

  return ret;
}

static bool uninstall_application(GDBusObjectManager* installed,
                                  const char* appid) {
  GList* objects = g_dbus_object_manager_get_objects(installed);
  GList* l;
  bool ret = false;

  for (l = objects; l; l = l->next) {
    GDBusObject* object = reinterpret_cast<GDBusObject*>(l->data);
    GDBusInterface* iface = g_dbus_object_get_interface(
        object,
        xwalk_installed_app_iface);
    if (!iface)
      continue;

    GDBusProxy* proxy = G_DBUS_PROXY(iface);

    GVariant* value = g_dbus_proxy_get_cached_property(proxy, "AppID");
    if (!value) {
      g_object_unref(iface);
      continue;
    }

    const char* id;
    g_variant_get(value, "s", &id);

    if (g_strcmp0(appid, id)) {
      g_object_unref(iface);
      continue;
    }

    GError* error = NULL;
    GVariant* result = g_dbus_proxy_call_sync(proxy, "Uninstall", NULL,
                                              G_DBUS_CALL_FLAGS_NONE,
                                              -1, NULL, &error);
    if (!result) {
      g_print("Uninstalling application failed: %s\n", error->message);
      g_error_free(error);
      g_object_unref(iface);
      ret = false;
      goto done;
    }

    g_object_unref(iface);
    ret = true;
    goto done;
  }

  g_print("Application ID '%s' could not be found\n", appid);

 done:
  g_list_free_full(objects, g_object_unref);

  return ret;
}

static void list_applications(GDBusObjectManager* installed,
                              GDBusObjectManager* running) {
  GList* objects = g_dbus_object_manager_get_objects(installed);
  GList* l;

  std::set<std::string> running_app_ids;
  if (running) {
    GList* running_objects = g_dbus_object_manager_get_objects(running);
    for (l = running_objects; l; l = l->next) {
      GDBusObject* object = reinterpret_cast<GDBusObject*>(l->data);
      GDBusInterface* iface =
          g_dbus_object_get_interface(object, xwalk_running_app_iface);
      if (!iface)
        continue;

      GDBusProxy* proxy = G_DBUS_PROXY(iface);
      GVariant* id_variant;
      id_variant = g_dbus_proxy_get_cached_property(proxy, "AppID");
      if (!id_variant) {
        g_object_unref(iface);
        continue;
      }

      const char* id;
      g_variant_get(id_variant, "s", &id);
      running_app_ids.insert(std::string(id));
      g_object_unref(iface);
    }
    g_list_free_full(running_objects, g_object_unref);
  }

  for (l = objects; l; l = l->next) {
    GDBusObject* object = reinterpret_cast<GDBusObject*>(l->data);
    GDBusInterface* iface = g_dbus_object_get_interface(
        object,
        xwalk_installed_app_iface);
    if (!iface)
      continue;

    GDBusProxy* proxy = G_DBUS_PROXY(iface);
    GVariant* id_variant;
    id_variant = g_dbus_proxy_get_cached_property(proxy, "AppID");
    if (!id_variant) {
      g_object_unref(iface);
      continue;
    }

    const char* id;
    g_variant_get(id_variant, "s", &id);

    GVariant* name_variant;
    name_variant = g_dbus_proxy_get_cached_property(proxy, "Name");
    if (!name_variant) {
      g_object_unref(iface);
      continue;
    }

    const char* name;
    g_variant_get(name_variant, "s", &name);

    if (running) {
      if (running_app_ids.find(std::string(id)) != running_app_ids.end()) {
        g_print("%s   \033[01;49;32mRunning\033[0m   %s\n", id, name);
      } else {
        g_print("%s   \033[02;49;37mNot run\033[0m   %s\n", id, name);
      }
    } else {
      g_print("%s\t%s\n", id, name);
    }

    g_object_unref(iface);
  }

  g_list_free_full(objects, g_object_unref);
}

int main(int argc, char* argv[]) {
  setlocale(LC_ALL, "");
  GError* error = NULL;
  GOptionContext* context;
  bool success;

#if !GLIB_CHECK_VERSION(2, 36, 0)
  // g_type_init() is deprecated on GLib since 2.36, Tizen has 2.32.
  g_type_init();
#endif

#if defined(OS_TIZEN)
  if (xwalk_tizen_check_user_app())
    exit(1);
#endif

  context = g_option_context_new("- Crosswalk Application Management");
  g_option_context_add_main_entries(context, entries, NULL);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_print("option parsing failed: %s\n", error->message);
    exit(1);
  }

  g_connection = get_session_bus_connection(&error);
  if (!g_connection) {
    fprintf(stderr, "Couldn't get the session bus connection: %s\n",
            error->message);
    exit(1);
  }

  GDBusObjectManager* installed_om =
      get_xwalk_object_manager(xwalk_installed_path);
  if (!installed_om)
    exit(1);

  GDBusObjectManager* running_om =
      get_xwalk_object_manager(xwalk_running_path);
  if (!running_om)
    exit(1);

  if (install_path) {
    success = install_application(install_path);
  } else if (uninstall_appid) {
    success = uninstall_application(installed_om, uninstall_appid);
  } else if (list_with_status) {
    g_print("Application ID                     Status    Application Name\n");
    g_print("-------------------------------------------------------------\n");
    list_applications(installed_om, running_om);
    g_print("-------------------------------------------------------------\n");
    success = true;
  } else {
    g_print("Application ID                       Application Name\n");
    g_print("-----------------------------------------------------\n");
    list_applications(installed_om, NULL);
    g_print("-----------------------------------------------------\n");
    success = true;
  }

  return success ? 0 : 1;
}
