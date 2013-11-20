// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "xwalk/application/browser/application_permission_service.h"

using xwalk::RuntimeContext;

namespace xwalk {
namespace application {

ApplicationPermissionService::ApplicationPermissionService(RuntimeContext* runtime_context)
    : runtime_context_(runtime_context) {
}

ApplicationPermissionService::~ApplicationPermissionService() {
}

//static
bool ApplicationPermissionService::CheckAPIAccessControl(std::string extension_name,
    std::string app_id, std::string api_name) {
  // check parameter
  // query permission db
  // return result
  return true;
}

}  // namespace application
}  // namespace xwalk
