// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_GROUP_MANAGER_H_
#define XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_GROUP_MANAGER_H_

#include <string>
#include <vector>

#include "xwalk/application/browser/security/permission_group.h"
#include "xwalk/application/common/permission_constants.h"

namespace xwalk {
namespace application {

class PermissionGroupManager {
 public:
  PermissionGroupManager();
  ~PermissionGroupManager();

  // If returned true, the perm_name contains the valid permission name
  // according to the given method name. Otherwise the perm_name will remain
  // untouched.
  bool GetPermissionName(const std::string& method_name,
                         std::string& perm_name);

  // Returns true if the permission name exists, otherwise false.
  bool IsValidPermissionName(const std::string& perm_name);

 private:
  std::vector<PermissionGroup*> group_register_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_GROUP_MANAGER_H_
