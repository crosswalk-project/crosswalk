// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_system_linux.h"

#include "base/command_line.h"
#include "xwalk/application/browser/application_service_provider_linux.h"
#include "xwalk/runtime/common/xwalk_switches.h"

namespace xwalk {
namespace application {

ApplicationSystemLinux::ApplicationSystemLinux(RuntimeContext* runtime_context)
    : ApplicationSystem(runtime_context) {
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kXWalkRunAsService)) {
    service_provider_.reset(
        new ApplicationServiceProviderLinux(application_service(),
                                            application_storage()));
  }
}

ApplicationSystemLinux::~ApplicationSystemLinux() {}

bool ApplicationSystemLinux::IsRunningAsService() const {
  return service_provider_;
}

}  // namespace application
}  // namespace xwalk
