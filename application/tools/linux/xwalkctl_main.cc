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

static GOptionEntry entries[] = {
  { "install", 'i', 0, G_OPTION_ARG_STRING, &install_path,
    "Path of the application to be installed/updated", "PATH" },
  { "uninstall", 'u', 0, G_OPTION_ARG_STRING, &uninstall_appid,
    "Uninstall the application with this appid", "APPID" },
  { NULL }
};

#if defined(SHARED_PROCESS_MODE)
namespace {

const char xwalk_service_name[] = "org.crosswalkproject.Runtime1";
const char xwalk_running_manager_iface[] =
    "org.crosswalkproject.Running.Manager1";
const dbus::ObjectPath kRunningManagerDBusPath("/running1");

}  // namespace

static void TerminateIfRunning(const std::string& app_id) {
  dbus::Bus::Options options;
#if defined(OS_TIZEN_MOBILE)
  options.bus_type = dbus::Bus::CUSTOM_ADDRESS;
  options.address.assign("unix:path=/run/user/app/dbus/user_bus_socket");
#endif
  scoped_refptr<dbus::Bus> bus(new dbus::Bus(options));
  dbus::ObjectProxy* app_proxy =
      bus->GetObjectProxy(xwalk_service_name, kRunningManagerDBusPath);
  if (!app_proxy)
    return;

  dbus::MethodCall method_call(
      xwalk_running_manager_iface, "TerminateIfRunning");
  dbus::MessageWriter writer(&method_call);
  writer.AppendString(app_id);

  app_proxy->CallMethodAndBlock(&method_call, 1000);
}
#endif

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
#if defined(OS_TIZEN)
    g_print("%s  %s\n",
            GetTizenAppId(app_data).c_str(),
            app_data->Name().c_str());
#else
    g_print("%s  %s\n", app_data->ID().c_str(), app_data->Name().c_str());
#endif
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
#if defined(OS_TIZEN)
    std::string crosswalk_app_id =
        xwalk::application::RawAppIdToCrosswalkAppId(uninstall_appid);
    uninstall_appid = strdup(crosswalk_app_id.c_str());
#endif
    success = installer->Uninstall(uninstall_appid);
  } else {
    success = list_applications(storage.get());
  }

  return success ? 0 : 1;
}
