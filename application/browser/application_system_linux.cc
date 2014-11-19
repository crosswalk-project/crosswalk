// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_system_linux.h"

#include "dbus/bus.h"
#include "xwalk/application/browser/application_service_provider_linux.h"
#include "xwalk/dbus/dbus_manager.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

namespace xwalk {
namespace application {

ApplicationSystemLinux::ApplicationSystemLinux(XWalkBrowserContext* context)
    : ApplicationSystem(context) {
  if (XWalkRunner::GetInstance()->shared_process_mode_enabled()) {
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
