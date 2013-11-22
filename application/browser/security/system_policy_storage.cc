// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/security/system_policy_storage.h"

namespace xwalk {
namespace application {

SystemPolicyStorage::SystemPolicyStorage() {
}

SystemPolicyStorage::~SystemPolicyStorage() {
}

StoredPermission SystemPolicyStorage::GetPermission(
    const std::string& app_id,
    const std::string& perm_name) {
  return ALLOW;
}

}  // namespace application
}  // namespace xwalk
