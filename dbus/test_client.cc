// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/dbus/test_client.h"

#include "dbus/bus.h"

namespace dbus {

TestClient::TestClient()
    : dbus_thread_("TestClient D-Bus Thread") {
  // Note that we need to create a thread here so that
  // dbus::Bus have a MessageLoop with type IO, that is necessary for its
  // operation.
  base::Thread::Options thread_options;
  thread_options.message_loop_type = base::MessageLoop::TYPE_IO;
  dbus_thread_.StartWithOptions(thread_options);

  dbus::Bus::Options options;
  options.dbus_task_runner = dbus_thread_.message_loop_proxy();
  bus_ = new dbus::Bus(options);
}

TestClient::~TestClient() {
  bus_->ShutdownOnDBusThreadAndBlock();
}

}  // namespace dbus
