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

#include "dbus/bus.h"
#include "dbus/message.h"
#include "dbus/object_proxy.h"

#include "xwalk/application/common/application_storage.h"
#include "xwalk/application/common/installer/package_installer.h"
#include "xwalk/application/tools/linux/dbus_connection.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#if defined(OS_TIZEN)
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/tools/linux/xwalk_tizen_user.h"
#endif

using xwalk::application::ApplicationData;
using xwalk::application::ApplicationStorage;
using xwalk::application::PackageInstaller;

static char* install_path;
static char* uninstall_appid;

static GDBusConnection* g_connection;
static gint debugging_port = -1;

static GOptionEntry entries[] = {
  { "install", 'i', 0, G_OPTION_ARG_STRING, &install_path,
    "Path of the application to be installed/updated", "PATH" },
  { "uninstall", 'u', 0, G_OPTION_ARG_STRING, &uninstall_appid,
    "Uninstall the application with this appid", "APPID" },
  { "debugging_port", 'd', 0, G_OPTION_ARG_INT, &debugging_port,
    "Enable remote debugging, port number 0 means to disable", NULL },
  { NULL }
};

namespace {

const char xwalk_service_name[] = "org.crosswalkproject.Runtime1";
const char xwalk_running_manager_iface[] =
    "org.crosswalkproject.Running.Manager1";
const char xwalk_running_manager_path[] = "/running1";
}  // namespace

#if defined(SHARED_PROCESS_MODE)
static void TerminateIfRunning(const std::string& app_id) {
  dbus::Bus::Options options;
#if defined(OS_TIZEN_MOBILE)
  options.bus_type = dbus::Bus::CUSTOM_ADDRESS;
  options.address.assign("unix:path=/run/user/app/dbus/user_bus_socket");
#endif
  scoped_refptr<dbus::Bus> bus(new dbus::Bus(options));
  dbus::ObjectProxy* app_proxy =
      bus->GetObjectProxy(
          xwalk_service_name,
          dbus::ObjectPath(std::string(xwalk_running_manager_path)));
  if (!app_proxy)
    return;

  dbus::MethodCall method_call(
      xwalk_running_manager_iface, "TerminateIfRunning");
  dbus::MessageWriter writer(&method_call);
  writer.AppendString(app_id);

  app_proxy->CallMethodAndBlock(&method_call, 1000);
}
#endif

static bool enable_remote_debugging(gint debugging_port) {
  GError* error = NULL;
  g_connection = get_session_bus_connection(&error);
  if (!g_connection) {
    fprintf(stderr, "Couldn't get the session bus connection: %s\n",
            error->message);
    exit(1);
  }
  GDBusProxy* running_proxy = g_dbus_proxy_new_sync(
      g_connection,
      G_DBUS_PROXY_FLAGS_NONE, NULL, xwalk_service_name,
      xwalk_running_manager_path, xwalk_running_manager_iface, NULL, &error);
  if (!running_proxy) {
    g_print("Couldn't create proxy for '%s': %s\n", xwalk_running_manager_iface,
            error->message);
    g_error_free(error);
    exit(1);
  }

  GVariant* result  = g_dbus_proxy_call_sync(
      running_proxy,
      "EnableRemoteDebugging",
      g_variant_new("(u)", debugging_port),
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  if (!result) {
    fprintf(stderr, "Couldn't call 'EnableRemoteDebugging' method: %s\n",
        error->message);
    exit(1);
  }

  int port = -1;
  g_variant_get(result, "(u)", &port);
  if ((port < 0) || (port != debugging_port)) {
    return false;
  } else if (port > 0) {
    fprintf(stderr, "Remote debugging enabled at port '%d'\n", port);
  } else {
    fprintf(stderr, "Remote debugging has been disabled\n");
  }
  return true;
}

bool list_applications(ApplicationStorage* storage) {
  std::vector<std::string> app_ids;
  if (!storage->GetInstalledApplicationIDs(app_ids))
    return false;

  g_print("Application ID                       Application Name\n");
  g_print("-----------------------------------------------------\n");
  for (unsigned i = 0; i < app_ids.size(); ++i) {
    scoped_refptr<ApplicationData> app_data =
        storage->GetApplicationData(app_ids.at(i));
    if (!app_data) {
      g_print("Failed to obtain app data for xwalk id: %s\n",
              app_ids.at(i).c_str());
      continue;
    }
    g_print("%s  %s\n", app_ids.at(i).c_str(), app_data->Name().c_str());
  }
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
    std::string app_id;
    const base::FilePath& path = base::FilePath(install_path);
    success = installer->Install(path, &app_id);
    if (!success && storage->Contains(app_id)) {
      g_print("trying to update %s\n", app_id.c_str());
      success = installer->Update(app_id, path);
    }
  } else if (uninstall_appid) {
#if defined(SHARED_PROCESS_MODE)
    TerminateIfRunning(uninstall_appid);
#endif
    success = installer->Uninstall(uninstall_appid);
  } else if (debugging_port >= 0) {
    // Deal with the case "xwalkctl -d PORT_NUMBER"
    success = enable_remote_debugging(debugging_port);
  } else {
    success = list_applications(storage.get());
  }

  return success ? 0 : 1;
}
