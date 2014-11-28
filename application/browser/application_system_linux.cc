// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_system_linux.h"

#include "base/command_line.h"
#include "dbus/bus.h"
#include "xwalk/application/browser/application_service_provider_linux.h"
#include "xwalk/dbus/dbus_manager.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_switches.h"

namespace xwalk {
namespace application {

ApplicationSystemLinux::ApplicationSystemLinux(RuntimeContext* runtime_context)
    : ApplicationSystem(runtime_context) {
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (!cmd_line->HasSwitch(switches::kXWalkDisableSharedProcessMode)) {
    service_provider_.reset(
        new ApplicationServiceProviderLinux(application_service(),
                                            dbus_manager().session_bus()));
  }
}

ApplicationSystemLinux::~ApplicationSystemLinux() {}

DBusManager& ApplicationSystemLinux::dbus_manager() {
  if (!dbus_manager_)
    dbus_manager_.reset(new DBusManager());
  return *dbus_manager_.get();
}

ApplicationServiceProviderLinux* ApplicationSystemLinux::service_provider() {
  return service_provider_.get();
}

}  // namespace application
}  // namespace xwalk
