// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_system.h"

#include "xwalk/application/browser/application_process_manager.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/runtime/browser/runtime_context.h"

using xwalk::RuntimeContext;

namespace xwalk_application {


ApplicationSystem::ApplicationSystem(RuntimeContext* runtime_context)
    : runtime_context_(runtime_context) {
  process_manager_.reset(new ApplicationProcessManager(runtime_context));
  application_service_.reset(new ApplicationService(runtime_context));
}

ApplicationSystem::~ApplicationSystem() {
}


}  // namespace xwalk_application
