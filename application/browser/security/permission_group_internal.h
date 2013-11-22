// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_GROUP_INTERNAL_H_
#define XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_GROUP_INTERNAL_H_

#include <string>

#include "base/compiler_specific.h"
#include "xwalk/application/browser/security/permission_group.h"

namespace xwalk {
namespace application {

class PermissionGroupInternal: public PermissionGroup {
 public:
  PermissionGroupInternal();
  ~PermissionGroupInternal();

  bool GetPermissionName(const std::string& method_name,
                         std::string& perm_name) OVERRIDE;

  bool IsValidPermissionName(const std::string& perm_name) OVERRIDE;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_GROUP_INTERNAL_H_
