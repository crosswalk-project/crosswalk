// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/security/session_policy_storage.h"

#include "base/logging.h"

namespace xwalk {
namespace application {

SessionPolicyStorage::SessionPolicyStorage() {
}

SessionPolicyStorage::~SessionPolicyStorage() {
}

StoredPermission SessionPolicyStorage::GetPermission(
    const std::string& app_id,
    const std::string& perm_name) {
  if (permission_map_.find(app_id) == permission_map_.end()) {
    return INVALID_STORED_PERM;
  } else {
    StoredPermissionMap& map = permission_map_[app_id];
    if (map.find(perm_name) == map.end())
      return INVALID_STORED_PERM;
    return map[perm_name];
  }
}

bool SessionPolicyStorage::SetPermission(const std::string& app_id,
                                         StoredPermissionMap& perm_map) {
  if (permission_map_.find(app_id) == permission_map_.end()) {
    permission_map_[app_id] = perm_map;
    return true;
  }
  StoredPermissionMap& map = permission_map_[app_id];
  for (StoredPermissionMap::iterator iter = perm_map.begin();
      iter != perm_map.end(); ++iter) {
    // Overwrite any existing policy.
    map[iter->first] = iter->second;
  }
  return true;
}

bool SessionPolicyStorage::RemovePermissionByID(const std::string& app_id) {
  if (permission_map_.find(app_id) == permission_map_.end()) {
    LOG(ERROR) << "Application ID does not exist.";
    return false;
  }
  permission_map_.erase(app_id);
  return true;
}

}  // namespace application
}  // namespace xwalk
