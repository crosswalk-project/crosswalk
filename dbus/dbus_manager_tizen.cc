// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/dbus/dbus_manager.h"

#include <glib.h>
#include <string>
#include "base/bind.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread.h"
#include "dbus/bus.h"

namespace xwalk {

DBusManager::DBusManager() {}

DBusManager::~DBusManager() {
  if (session_bus_)
    session_bus_->ShutdownOnDBusThreadAndBlock();
}

scoped_refptr<dbus::Bus> DBusManager::session_bus() {
  if (!session_bus_) {
    base::Thread::Options thread_options;
    thread_options.message_loop_type = base::MessageLoop::TYPE_IO;
    std::string thread_name = "Crosswalk D-Bus thread";
    dbus_thread_.reset(new base::Thread(thread_name.c_str()));
    dbus_thread_->StartWithOptions(thread_options);

    dbus::Bus::Options options;
    options.dbus_task_runner = dbus_thread_->message_loop_proxy();

    // On Tizen 2.x DBUS_SESSION_ADDRESS points to a wrong path, so we set
    // the correct one here.
    options.bus_type = dbus::Bus::CUSTOM_ADDRESS;
    options.address = base::StringPrintf(
        "unix:path=/run/user/%s/dbus/user_bus_socket", g_get_user_name());

    session_bus_ = new dbus::Bus(options);
  }

  return session_bus_;
}

}  // namespace xwalk
