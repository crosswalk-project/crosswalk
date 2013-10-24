// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_daemon.h"

#include "dbus/bus.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/runtime/browser/runtime_context.h"

using xwalk::RuntimeContext;

namespace xwalk {
namespace application {

ApplicationDaemon::ApplicationDaemon(xwalk::RuntimeContext* runtime_context) {
}

ApplicationDaemon::~ApplicationDaemon() {
}

bool ApplicationDaemon::Daemonize() {
  LOG(INFO) << "Daemonize ...";
  return true;
}

}  // namespace application
}  // namespace xwalk
