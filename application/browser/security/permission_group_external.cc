// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/security/permission_group_external.h"

namespace xwalk {
namespace application {

// TODO(Bai): Complete this class implementation.
PermissionGroupExternal::PermissionGroupExternal() {
}

PermissionGroupExternal::~PermissionGroupExternal() {
}

bool PermissionGroupExternal::GetPermissionName(
    const std::string& method_name,
    std::string& perm_name) {
  return false;
}

bool PermissionGroupExternal::IsValidPermissionName(
    const std::string& perm_name) {
  return false;
}

}  // namespace application
}  // namespace xwalk
