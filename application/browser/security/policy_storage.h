// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_SECURITY_POLICY_STORAGE_H_
#define XWALK_APPLICATION_BROWSER_SECURITY_POLICY_STORAGE_H_

#include <map>
#include <string>

#include "base/compiler_specific.h"
#include "xwalk/application/common/permission_constants.h"

namespace xwalk {
namespace application {

typedef std::map<std::string, StoredPermission> StoredPermissionMap;

class PolicyStorage {
 public:
  PolicyStorage() {}
  virtual ~PolicyStorage() {}

  virtual StoredPermission GetPermission(const std::string& app_id,
                                         const std::string& perm_name) = 0;

  virtual bool SetPermission(const std::string& app_id,
                             StoredPermissionMap& perm_map) = 0;

  bool SetSinglePermission(const std::string& app_id,
                           const std::string& perm_name,
                           const StoredPermission perm) {
    StoredPermissionMap perm_map;
    perm_map[perm_name] = perm;
    return SetPermission(app_id, perm_map);
  }
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_SECURITY_POLICY_STORAGE_H_
