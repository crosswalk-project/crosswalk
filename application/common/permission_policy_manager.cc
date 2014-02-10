// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/permission_policy_manager.h"

namespace xwalk {
namespace application {

PermissionPolicyManager::PermissionPolicyManager() {
  // TODO(Bai): Initialize this class with the external policy file.
}

PermissionPolicyManager::~PermissionPolicyManager() {
}

StoredPermission PermissionPolicyManager::GetPermission(
    const std::string& app_id, const std::string& permission_name) const {
  // TODO(Bai): The filtering logic will be implemented after the policy
  // file format is defined.
  return ALLOW;
}

}  // namespace application
}  // namespace xwalk
