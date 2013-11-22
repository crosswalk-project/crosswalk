// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_SECURITY_PERSISTENT_POLICY_STORAGE_H_
#define XWALK_APPLICATION_BROWSER_SECURITY_PERSISTENT_POLICY_STORAGE_H_

#include <string>

#include "xwalk/application/browser/security/policy_storage.h"

namespace xwalk {
namespace application {

class PersistentPolicyStorage: public PolicyStorage {
 public:
  PersistentPolicyStorage();
  ~PersistentPolicyStorage() OVERRIDE;

  StoredPermission GetPermission(const std::string& app_id,
                                 const std::string& perm_name) OVERRIDE;

  bool SetPermission(const std::string& app_id,
                     StoredPermissionMap& perm_map) OVERRIDE;

  bool RemovePermissionByID(const std::string& app_id);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_SECURITY_PERSISTENT_POLICY_STORAGE_H_
