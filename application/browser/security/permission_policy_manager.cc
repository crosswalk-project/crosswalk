// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/security/permission_policy_manager.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "xwalk/application/common/permission_constants.h"

namespace xwalk {
namespace application {

PermissionPolicyManager::PermissionPolicyManager() {
  session_policy_.reset(new SessionPolicyStorage());
  persistent_policy_.reset(new PersistentPolicyStorage());
  system_policy_.reset(new SystemPolicyStorage());
}

PermissionPolicyManager::~PermissionPolicyManager() {
}

void PermissionPolicyManager::GetRuntimePerm(
    const std::string& app_id,
    const std::string& perm_name,
    OnRuntimePermCallback callback) {
  // Consult session policy first.
  StoredPermission perm = session_policy_->GetPermission(app_id, perm_name);
  if (perm != INVALID_STORED_PERM) {
    // "ASK" should not be in the session storage.
    DCHECK(perm != ASK);
    if (perm == ALLOW) {
      base::MessageLoop::current()->PostTask(
          FROM_HERE, base::Bind(callback, DENY_SESSION));
      return;
    }
    if (perm == DENY) {
      base::MessageLoop::current()->PostTask(
                FROM_HERE, base::Bind(callback, DENY_SESSION));
      return;
    }
    NOTREACHED();
  }
  // Then, consult the persistent policy storage.
  perm = persistent_policy_->GetPermission(app_id, perm_name);
  // Permission not found in persistent permission table, normally this should
  // not happen because all the permission needed by the application should be
  // contained in its manifest, so it also means that the application is asking
  // for something wasn't allowed.
  if (perm == INVALID_STORED_PERM) {
    base::MessageLoop::current()->PostTask(
              FROM_HERE, base::Bind(callback, INVALID_RUNTIME_PERM));
    return;
  }
  if (perm == ASK) {
    // TODO(Bai): We needed to pop-up a dialog asking user to chose one from
    // either allow/deny for session/one shot/forever. Then, we need to update
    // the session policy table accordingly.
  }
  if (perm == ALLOW) {
    base::MessageLoop::current()->PostTask(
                  FROM_HERE, base::Bind(callback, ALLOW_FOREVER));
    return;
  }
  if (perm == DENY) {
    base::MessageLoop::current()->PostTask(
                  FROM_HERE, base::Bind(callback, DENY_FOREVER));
    return;
  }
  NOTREACHED();
}

bool PermissionPolicyManager::PermInstall(
    const std::string& app_id,
    const std::vector<std::string>& perm_name_list) {
  StoredPermissionMap perm_map;
  StoredPermission perm;
  for (std::vector<std::string>::const_iterator it = perm_name_list.begin();
      it != perm_name_list.end(); ++it) {
    perm = system_policy_->GetPermission(app_id, *it);
    if (perm == INVALID_STORED_PERM) {
      return false;
    }
    // Could either be ASK, ALLOW or DENY
    perm_map[*it] = perm;
  }
  // It's transaction-protected.
  return persistent_policy_->SetPermission(app_id, perm_map);
}

bool PermissionPolicyManager::PermUninstall(const std::string& app_id) {
  // First, remove the session policy.
  if (!TerminateSession(app_id))
    return false;
  // Then, clean up the persistent storage.
  return persistent_policy_->RemovePermissionByID(app_id);
}

bool PermissionPolicyManager::TerminateSession(const std::string& app_id) {
  return session_policy_->RemovePermissionByID(app_id);
}

}  // namespace application
}  // namespace xwalk
