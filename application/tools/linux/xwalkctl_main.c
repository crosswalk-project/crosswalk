// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>
#include <stdbool.h>

#include <glib.h>
#include <gio/gio.h>

static const char* xwalk_service_name = "org.crosswalkproject";
static const char* xwalk_installed_path = "/installed";
static const char* xwalk_installed_iface =
    "org.crosswalkproject.InstalledApplicationsRoot";
static const char* xwalk_installed_app_iface =
    "org.crosswalkproject.InstalledApplication";

static char* install_path;
static char* uninstall_appid;

static GOptionEntry entries[] =
{
  { "install", 'i', 0, G_OPTION_ARG_STRING, &install_path,
    "Path of the application to be installed", "PATH" },
  { "uninstall", 'u', 0, G_OPTION_ARG_STRING, &uninstall_appid,
    "Uninstall the application with this appid", "APPID" },
  { NULL }
};

static bool install_application(const char* path)
{
  GError* error = NULL;
  GDBusProxy* proxy;
  bool ret;

  proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
		  G_DBUS_PROXY_FLAGS_NONE, NULL, xwalk_service_name,
		  xwalk_installed_path, xwalk_installed_iface, NULL, &error);
  if (!proxy) {
    g_print("Couldn't create proxy for '%s': %s\n", xwalk_installed_iface,
            error->message);
    g_error_free(error);
    ret = false;
    goto done;
  }

  GVariant* result = g_dbus_proxy_call_sync(proxy, "Install",
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
  g_print("Application installed with path '%s'\n", object_path);
  g_variant_unref(result);

  ret = true;

done:
  if (proxy)
    g_object_unref(proxy);

  return ret;
}

static bool uninstall_application(GDBusObjectManager* installed,
                                  const char* appid)
{
  GList* objects = g_dbus_object_manager_get_objects(installed);
  GList* l;
  bool ret = false;

  for (l = objects; l; l = l->next) {
    GDBusObject* object = l->data;
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

static void list_applications(GDBusObjectManager* installed) {
  GList* objects = g_dbus_object_manager_get_objects(installed);
  GList* l;

  for (l = objects; l; l = l->next) {
    GDBusObject* object = l->data;
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

    g_print("%s\t%s\n", id, name);

    g_object_unref(iface);
  }

  g_list_free_full(objects, g_object_unref);
}

int main(int argc, char* argv[]) {
  GError* error = NULL;
  GOptionContext* context;
  GMainLoop* mainloop;
  int err = 0;
  bool success;

#if !GLIB_CHECK_VERSION(2, 36, 0)
  // g_type_init() is deprecated on GLib since 2.36, Tizen has 2.32.
  g_type_init();
#endif

  context = g_option_context_new("- Crosswalk Application Management");
  g_option_context_add_main_entries(context, entries, NULL);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_print("option parsing failed: %s\n", error->message);
    exit(1);
  }

  GDBusObjectManager* installed_om = g_dbus_object_manager_client_new_for_bus_sync(
      G_BUS_TYPE_SESSION, G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
      xwalk_service_name, xwalk_installed_path,
      NULL, NULL, NULL, NULL, NULL);
  if (!installed_om) {
    g_print("Service '%s' could not be reached\n", xwalk_service_name);
    exit(1);
  }

  if (install_path) {
    success = install_application(install_path);
  } else if (uninstall_appid) {
    success = uninstall_application(installed_om, uninstall_appid);
  } else {
    g_print("Application ID                       Application Name\n");
    g_print("-----------------------------------------------------\n");
    list_applications(installed_om);
    g_print("-----------------------------------------------------\n");
    success = true;
  }

  return success ? 0 : 1;
}
