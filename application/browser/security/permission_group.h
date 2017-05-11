// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_GROUP_H_
#define XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_GROUP_H_

#include <string>

#include "xwalk/application/common/permission_constants.h"

namespace xwalk {
namespace application {

class PermissionGroup {
 public:
  PermissionGroup() {}
  virtual ~PermissionGroup() {}

  // If returned true, the perm_name contains the permission name
  // according to the provided method name, otherwise the perm_name
  // is untouched.
  virtual bool GetPermissionName(const std::string& method_name,
                                 std::string& perm_name) = 0;

  virtual bool IsValidPermissionName(const std::string& perm_name) = 0;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_GROUP_H_
