// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unistd.h>

#include <string>
#include <utility>
#include <vector>

#include "dbus/object_proxy.h"

#include "base/logging.h"

#include "xwalk/application/tools/linux/xwalk_launcher.h"

namespace {

const char xwalk_service_name[] = "org.crosswalkproject.Runtime1";
const char xwalk_running_path[] = "/running1";
const char xwalk_running_manager_iface[] =
    "org.crosswalkproject.Running.Manager1";
const char xwalk_running_app_iface[] =
    "org.crosswalkproject.Running.Application1";

}  // namespace

XWalkLauncher::XWalkLauncher(bool query_running, base::MessageLoop* main_loop)
    : ep_launcher_(nullptr),
      query_running_(query_running),
      main_loop_(main_loop) {
  base::Thread::Options thread_options;
  thread_options.message_loop_type = base::MessageLoop::TYPE_IO;
  dbus_thread_.reset(new base::Thread("Crosswalk D-Bus thread"));
  dbus_thread_->StartWithOptions(thread_options);

  dbus::Bus::Options options;
#if defined (OS_TIZEN_MOBILE)
  options.bus_type = dbus::Bus::CUSTOM_ADDRESS;
  options.address.assign("unix:path=/run/user/app/dbus/user_bus_socket");
#endif
  options.bus_type = dbus::Bus::SESSION;
  options.connection_type = dbus::Bus::PRIVATE;
  options.dbus_task_runner = dbus_thread_->message_loop_proxy();
  dbus::Bus* bus = new dbus::Bus(options);
  bus->Connect();
  bus->GetManagedObjects();

  dbus_object_manager_.reset(new DBusObjectManager(bus, main_loop));
  dbus_object_manager_->SetObserver(this);
}

XWalkLauncher::~XWalkLauncher() {
}

int XWalkLauncher::Launch(const std::string& appid_or_url, bool fullscreen,
                          bool remote_debugging, int argc, char* argv[]) {
  appid_or_url_ = appid_or_url;
  fullscreen_ = fullscreen;
  remote_debugging_ = remote_debugging;

  // Query app.
  if (query_running_) {
    return dbus_object_manager_->IsApplicationRunning(appid_or_url_);
  }

  return !LaunchApplication();
}

int XWalkLauncher::LaunchApplication() {
  ep_launcher_.reset(new XWalkExtensionProcessLauncher());

  launcher_pid_ = getpid();
  if (!dbus_object_manager_->Launch(appid_or_url_, launcher_pid_,
                                    fullscreen_, remote_debugging_))
    return 1;
  return InitExtensionProcessChannel();
}

bool XWalkLauncher::InitExtensionProcessChannel() {
  if (ep_launcher_->is_started())
    return false;

  // Need to call method via DBus to get EP channel
  std::pair<std::string, int> fd = dbus_object_manager_->GetEPChannel();
  if (fd.first.empty() || fd.second < 0)
    return false;
  ep_launcher_->Launch(fd.first, fd.second);
  return true;
}

void XWalkLauncher::OnEPChannelCreated() {
  InitExtensionProcessChannel();
}
