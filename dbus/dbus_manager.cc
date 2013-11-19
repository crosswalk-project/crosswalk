// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/dbus/dbus_manager.h"

#include <string>
#include "base/bind.h"
#include "base/threading/thread.h"
#include "dbus/bus.h"

namespace xwalk {

DBusManager::DBusManager(const std::string& service_name)
    : weak_factory_(this),
      service_name_(service_name) {}

DBusManager::~DBusManager() {
  session_bus_->ShutdownOnDBusThreadAndBlock();
}

void DBusManager::Initialize(const base::Closure& on_success_callback) {
  on_success_callback_ = on_success_callback;

  base::Thread::Options thread_options;
  thread_options.message_loop_type = base::MessageLoop::TYPE_IO;
  std::string thread_name = "Crosswalk D-Bus thread (" + service_name_ + ")";
  dbus_thread_.reset(new base::Thread(thread_name.c_str()));
  dbus_thread_->StartWithOptions(thread_options);

  dbus::Bus::Options options;
  options.dbus_task_runner = dbus_thread_->message_loop_proxy();
  session_bus_ = new dbus::Bus(options);

  session_bus_->RequestOwnership(
      service_name_, dbus::Bus::REQUIRE_PRIMARY,
      base::Bind(&DBusManager::OnNameOwned, weak_factory_.GetWeakPtr()));
}

scoped_refptr<dbus::Bus> DBusManager::session_bus() {
  return session_bus_;
}

void DBusManager::OnNameOwned(const std::string& service_name, bool success) {
  if (!success) {
    LOG(ERROR) << "Could not own DBus service name '" << service_name << "'.";
    return;
  }
  on_success_callback_.Run();
}

}  // namespace xwalk
