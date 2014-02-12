// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_PERMISSION_POLICY_MANAGER_H_
#define XWALK_APPLICATION_COMMON_PERMISSION_POLICY_MANAGER_H_

#include <string>

#include "base/basictypes.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/permission_types.h"

namespace xwalk {
namespace application {

// During application installation, the permission stored in the manifest
// has to be filtered and converted into the stored permission list. During
// this process, the permission policy manager is responsible for generating
// the appropriate permission value based on either internal or external
// (OEM) policies.
// For example, we may have ["bluetooth"] in the manifest, but according
// to the policies, any bluetooth request should be confirmed by user,
// therefore, instead of writing "bluetooth:ALLOW", we write
// "bluetooth:PROMPT" into database.
class PermissionPolicyManager {
 public:
  PermissionPolicyManager();
  ~PermissionPolicyManager();
  // Permissions listed in manifest would be a little different from what
  // stored in database. In manifest we have permissions stored in a JSON
  // array, like ["bluetooth","contacts"], but in database, key-value pairs
  // are used, like "bluetooth:ALLOW; contacts:PROMPT". A policy based
  // conversion should be done during installation, and this also means that
  // the function would only be used during installation.
  bool InitApplicationPermission(ApplicationData* app_data);

 private:
  StoredPermission GetPermission(const std::string& app_id,
                                 const std::string& permission_name) const;
  DISALLOW_COPY_AND_ASSIGN(PermissionPolicyManager);
};

}  // namespace application
}  // namespace xwalk
#endif  // XWALK_APPLICATION_COMMON_PERMISSION_POLICY_MANAGER_H_
