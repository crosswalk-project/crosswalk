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

bool PermissionPolicyManager::InitApplicationPermission(
    ApplicationData* app_data) {
  app_data->ClearPermissions();
  PermissionSet permissions = app_data->GetManifestPermissions();
  if (permissions.empty())
    return true;
  // Convert manifest permissions to stored permissions.
  StoredPermission perm = UNDEFINED_STORED_PERM;
  for (PermissionSet::const_iterator iter = permissions.begin();
      iter != permissions.end(); ++iter) {
    perm = GetPermission(app_data->ID(), *iter);
    if (perm == UNDEFINED_STORED_PERM) {
      // If there are something added before we run into this undefined
      // permission item, clear all the previous ones.
      app_data->ClearPermissions();
      LOG(ERROR) << "Invalid permission found: " << *iter;
      return false;
    }
    app_data->SetPermission(*iter, perm);
  }
  return true;
}

}  // namespace application
}  // namespace xwalk
