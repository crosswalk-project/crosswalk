// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_service.h"

#include "xwalk/runtime/browser/runtime_context.h"

using xwalk::RuntimeContext;

namespace xwalk{
namespace application {

ApplicationService::ApplicationService(RuntimeContext* runtime_context)
    : runtime_context_(runtime_context) {
}

ApplicationService::~ApplicationService() {
}

}  // namespace application
}  // namespace xwalk
