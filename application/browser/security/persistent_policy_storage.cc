// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/security/persistent_policy_storage.h"

namespace xwalk {
namespace application {

PersistentPolicyStorage::PersistentPolicyStorage() {
}

PersistentPolicyStorage::~PersistentPolicyStorage() {
}

StoredPermission PersistentPolicyStorage::GetPermission(
    const std::string& app_id,
    const std::string& perm_name) {
  return INVALID_STORED_PERM;
}

bool PersistentPolicyStorage::SetPermission(const std::string& app_id,
                                            StoredPermissionMap& perm_map) {
  return false;
}

bool PersistentPolicyStorage::RemovePermissionByID(const std::string& app_id) {
  return false;
}

}  // namespace application
}  // namespace xwalk
