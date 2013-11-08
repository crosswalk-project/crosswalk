// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_service_provider.h"

#include "xwalk/runtime/browser/runtime_context.h"

using xwalk::RuntimeContext;

namespace xwalk {
namespace application {

ApplicationServiceProvider::ApplicationServiceProvider(
        xwalk::RuntimeContext* runtime_context) {
}

ApplicationServiceProvider::~ApplicationServiceProvider() {
}

bool ApplicationServiceProvider::Start() {
  NOTIMPLEMENTED();
  return false;
}

}  // namespace application
}  // namespace xwalk
