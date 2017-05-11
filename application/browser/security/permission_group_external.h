// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_GROUP_EXTERNAL_H_
#define XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_GROUP_EXTERNAL_H_

#include <string>

#include "base/compiler_specific.h"
#include "xwalk/application/browser/security/permission_group.h"

namespace xwalk {
namespace application {

class PermissionGroupExternal: public PermissionGroup {
 public:
  PermissionGroupExternal();
  ~PermissionGroupExternal();

  bool GetPermissionName(const std::string& method_name,
                         std::string& perm_name) OVERRIDE;

  bool IsValidPermissionName(const std::string& perm_name) OVERRIDE;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_GROUP_EXTERNAL_H_
