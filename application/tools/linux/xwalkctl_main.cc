// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <glib.h>
#include <gio/gio.h>

#include <limits>

#include "base/at_exit.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"

#include "xwalk/application/common/tizen/application_storage.h"
#include "xwalk/runtime/common/xwalk_paths.h"

#include "dbus/bus.h"
#include "dbus/message.h"
#include "dbus/object_proxy.h"

#if defined(OS_TIZEN)
#include "xwalk/application/tools/tizen/xwalk_tizen_user.h"
#endif

using xwalk::application::ApplicationData;
using xwalk::application::ApplicationStorage;

namespace {

const gint debugging_port_not_set = std::numeric_limits<gint>::min();

gint debugging_port = debugging_port_not_set;
GOptionEntry entries[] = {
  { "debugging_port", 'd', 0, G_OPTION_ARG_INT, &debugging_port,
    "Enable remote debugging, port number 0 means to disable", NULL },
  { NULL }
};

const char kServiceName[] = "org.crosswalkproject.Runtime1";
const char kRunningManagerIface[] =
    "org.crosswalkproject.Running.Manager1";
const dbus::ObjectPath kRunningManagerDBusPath("/running1");

bool EnableRemoteDebugging(int port) {
  if (port < 0) {
    g_print("Remote debugging port cannot be negative\n");
    return false;
  }
  if (port >= 65535) {
    DLOG(ERROR) << "Invalid http debugger port number " << port;
    return false;
  }

  dbus::Bus::Options options;
#if defined(OS_TIZEN_MOBILE)
  options.bus_type = dbus::Bus::CUSTOM_ADDRESS;
  options.address.assign("unix:path=/run/user/app/dbus/user_bus_socket");
#endif
  scoped_refptr<dbus::Bus> bus(new dbus::Bus(options));
  dbus::ObjectProxy* app_proxy =
      bus->GetObjectProxy(
          kServiceName,
          kRunningManagerDBusPath);
  if (!app_proxy) {
    DLOG(ERROR) << "Failed to get application proxy.";
    return false;
  }

  dbus::MethodCall method_call(
      kRunningManagerIface, "EnableRemoteDebugging");
  dbus::MessageWriter writer(&method_call);
  writer.AppendUint32(port);

  app_proxy->CallMethodAndBlock(&method_call, 1000);

  if (!port)
    g_print("Remote debugging has been disabled \n");
  else
    g_print("Remote debugging enabled at port %d \n", port);
  return true;
}

bool list_applications(ApplicationStorage* storage) {
  std::vector<std::string> app_ids;
  if (!storage->GetInstalledApplicationIDs(app_ids))
    return false;

  g_print("Application ID                       Application Name\n");
  g_print("-----------------------------------------------------\n");
  for (auto id : app_ids) {
    scoped_refptr<ApplicationData> app_data = storage->GetApplicationData(id);
    if (!app_data.get()) {
      g_print("Failed to obtain app data for xwalk id: %s\n",
              id.c_str());
      continue;
    }
    g_print("%s  %s\n", id.c_str(), app_data->Name().c_str());
  }
  g_print("-----------------------------------------------------\n");

  return true;
}

}  // namespace

int main(int argc, char* argv[]) {
#if defined(OS_TIZEN)
  if (xwalk_tizen_check_group_users())
    return 1;
#endif
  GOptionContext* context = g_option_context_new("- Crosswalk Setter");
  g_option_context_add_main_entries(context, entries, NULL);
  GError* error = nullptr;
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_print("option parsing failed: %s\n", error->message);
    g_option_context_free(context);
    return 1;
  }
  g_option_context_free(context);
  base::AtExitManager at_exit;

  if (debugging_port != debugging_port_not_set) {
    if (!EnableRemoteDebugging(static_cast<int>(debugging_port)))
      return 1;
  } else {
    // FIXME : there should be a way to get application Id from platform, so
    // the below code should not be needed.
    xwalk::RegisterPathProvider();
    base::FilePath data_path;
    PathService::Get(xwalk::DIR_DATA_PATH, &data_path);
    scoped_ptr<ApplicationStorage> storage(new ApplicationStorage(data_path));
    if (!list_applications(storage.get()))
      return 1;
  }

  return 0;
}
