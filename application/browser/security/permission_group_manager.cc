// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/security/permission_group_manager.h"

#include "base/logging.h"
#include "xwalk/application/browser/security/permission_group_external.h"
#include "xwalk/application/browser/security/permission_group_internal.h"

namespace xwalk {
namespace application {

PermissionGroupManager::PermissionGroupManager() {
  group_register_.push_back(new PermissionGroupInternal());
  group_register_.push_back(new PermissionGroupExternal());
}

PermissionGroupManager::~PermissionGroupManager() {
  for (std::vector<PermissionGroup*>::const_iterator iter
      = group_register_.begin(); iter != group_register_.end(); ++iter) {
    delete *iter;
  }
}

bool PermissionGroupManager::GetPermissionName(const std::string& method_name,
                                               std::string& perm_name) {
  for (std::vector<PermissionGroup*>::const_iterator iter
      = group_register_.begin(); iter != group_register_.end(); ++iter) {
    if ((*iter)->GetPermissionName(method_name, perm_name))
      return true;
  }
  return false;
}

bool PermissionGroupManager::IsValidPermissionName(
    const std::string& perm_name) {
}

}  // namespace application
}  // namespace xwalk
