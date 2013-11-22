// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_SECURITY_SESSION_POLICY_STORAGE_H_
#define XWALK_APPLICATION_BROWSER_SECURITY_SESSION_POLICY_STORAGE_H_

#include <map>
#include <string>

#include "xwalk/application/browser/security/policy_storage.h"

namespace xwalk {
namespace application {
// Session policies are stored in a map, something like
// [ [APP1,[bluetooth,ALLOW]], [APP2,[contact,DENY]] ...]
class SessionPolicyStorage: public PolicyStorage {
 public:
  SessionPolicyStorage();
  ~SessionPolicyStorage() OVERRIDE;

  StoredPermission GetPermission(const std::string& app_id,
                                 const std::string& perm_name) OVERRIDE;

  bool SetPermission(const std::string& app_id,
                     StoredPermissionMap& perm_map) OVERRIDE;

  bool RemovePermissionByID(const std::string& app_id);
 private:
  std::map<std::string, StoredPermissionMap> permission_map_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_SECURITY_SESSION_POLICY_STORAGE_H_
