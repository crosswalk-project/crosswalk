// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_PERMISSION_TYPES_H_
#define XWALK_APPLICATION_COMMON_PERMISSION_TYPES_H_

#include <map>
#include <set>
#include <string>

#include "base/callback.h"

// TODO(Bai): Whenever this file is edited, make sure the content of
// extensions/common/xwalk_extension_permission_types.h is aligned.
// See comments there for more information.
namespace xwalk {
namespace application {

enum PermissionType {
  SESSION_PERMISSION,
  PERSISTENT_PERMISSION,
};

enum PromptType {
  UNSET_PROMPT_TYPE = -1,
  PROMPT_ONESHOT = 0,
  PROMPT_SESSION,
  PROMPT_BLANKET
};

enum RuntimePermission {
  ALLOW_ONCE = 0,
  ALLOW_SESSION,
  ALLOW_ALWAYS,
  DENY_ONCE,
  DENY_SESSION,
  DENY_ALWAYS,
  UNDEFINED_RUNTIME_PERM,
};

typedef base::Callback<void(RuntimePermission)> PermissionCallback;

enum StoredPermission {
  ALLOW = 0,
  DENY,
  PROMPT,
  UNDEFINED_STORED_PERM,
};

typedef std::map<std::string, StoredPermission> StoredPermissionMap;
typedef std::set<std::string> PermissionSet;

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_PERMISSION_TYPES_H_
