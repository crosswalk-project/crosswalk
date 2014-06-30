// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <glib.h>
#include <gio/gio.h>
#include <locale.h>

#include "base/at_exit.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"

#include "xwalk/application/common/application_storage.h"
#include "xwalk/application/common/installer/package_installer.h"
#include "xwalk/application/tools/linux/dbus_connection.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#if defined(OS_TIZEN)
#include "xwalk/application/tools/linux/xwalk_tizen_user.h"
#endif

using xwalk::application::ApplicationData;
using xwalk::application::ApplicationStorage;
using xwalk::application::PackageInstaller;

static char* install_path;
static char* uninstall_appid;

static GOptionEntry entries[] = {
  { "install", 'i', 0, G_OPTION_ARG_STRING, &install_path,
    "Path of the application to be installed/updated", "PATH" },
  { "uninstall", 'u', 0, G_OPTION_ARG_STRING, &uninstall_appid,
    "Uninstall the application with this appid", "APPID" },
  { NULL }
};

static bool uninstall_application(const char* appid) {
  static const char* xwalk_service_name = "org.crosswalkproject.Runtime1";
  static const char* xwalk_installed_path = "/installed1";
  static const char* xwalk_installed_app_iface =
      "org.crosswalkproject.Installed.Application1";

  GError* error = NULL;
  GDBusConnection* connection = get_session_bus_connection(&error);
  if (!connection) {
    fprintf(stderr, "Couldn't get the session bus connection: %s\n",
            error->message);
    exit(1);
  }

  GDBusObjectManager* installed_om = g_dbus_object_manager_client_new_sync(
      connection, G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
      xwalk_service_name, xwalk_installed_path,
      NULL, NULL, NULL, NULL, &error);
  if (!installed_om) {
    g_print("Service '%s' could not be reached: %s\n", xwalk_service_name,
            error->message);
    exit(1);
  }

  // GDBus may return a valid ObjectManager even if it fails to auto activate
  // the proper service name. We should check 'name-owner' property to make sure
  // the service is working. See GDBusObjectManager documentation.
  gchar* name_owner = NULL;
  g_object_get(installed_om, "name-owner", &name_owner, NULL);
  if (!name_owner) {
    g_print("Service '%s' could not be activated.\n", xwalk_service_name);
    exit(1);
  }
  g_free(name_owner);

  GList* objects = g_dbus_object_manager_get_objects(installed_om);
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

bool list_applications(ApplicationStorage* storage) {
  ApplicationData::ApplicationDataMap apps;
  if (!storage->GetInstalledApplications(apps))
    return false;

  g_print("Application ID                       Application Name\n");
  g_print("-----------------------------------------------------\n");
  ApplicationData::ApplicationDataMap::const_iterator it;
  for (it = apps.begin(); it != apps.end(); ++it)
    g_print("%s  %s\n", it->first.c_str(), it->second->Name().c_str());
  g_print("-----------------------------------------------------\n");

  return true;
}

int main(int argc, char* argv[]) {
  setlocale(LC_ALL, "");
  GError* error = NULL;
  GOptionContext* context;
  bool success = false;

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

  base::AtExitManager at_exit;
  base::FilePath data_path;
  xwalk::RegisterPathProvider();
  PathService::Get(xwalk::DIR_DATA_PATH, &data_path);
  scoped_ptr<ApplicationStorage> storage(new ApplicationStorage(data_path));
  scoped_ptr<PackageInstaller> installer =
      PackageInstaller::Create(storage.get());

  if (install_path) {
    std::string id;
    success = installer->Install(base::FilePath(install_path), &id);
  } else if (uninstall_appid) {
    success = uninstall_application(uninstall_appid);
  } else {
    success = list_applications(storage.get());
  }

  return success ? 0 : 1;
}
