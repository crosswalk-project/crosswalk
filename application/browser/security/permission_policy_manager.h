// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_POLICY_MANAGER_H_
#define XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_POLICY_MANAGER_H_

#include <string>
#include <vector>

#include "base/callback.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/application/common/permission_constants.h"
#include "xwalk/application/browser/security/persistent_policy_storage.h"
#include "xwalk/application/browser/security/session_policy_storage.h"
#include "xwalk/application/browser/security/system_policy_storage.h"


namespace xwalk {
namespace application {

typedef base::Callback<void (const RuntimePermission&)> OnRuntimePermCallback;

class PermissionPolicyManager {
 public:
  PermissionPolicyManager();
  ~PermissionPolicyManager();

  void GetRuntimePerm(const std::string& app_id,
                      const std::string& perm_name,
                      OnRuntimePermCallback callback);

  // We take the permission list from installer, indicating the permissions
  // needed by the application, i.e. written in its manifest. But we need
  // verify it against the system policy first.
  // Note that, the permission name should be verified before calling this
  // function.
  bool PermInstall(const std::string& app_id,
                   const std::vector<std::string>& perm_name_list);

  bool PermUninstall(const std::string& app_id);

  bool TerminateSession(const std::string& app_id);
 private:
  scoped_ptr<SessionPolicyStorage> session_policy_;
  scoped_ptr<PersistentPolicyStorage> persistent_policy_;
  scoped_ptr<SystemPolicyStorage> system_policy_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_SECURITY_PERMISSION_POLICY_MANAGER_H_
