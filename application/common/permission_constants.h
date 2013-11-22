// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_PERMISSION_CONSTANTS_H_
#define XWALK_APPLICATION_COMMON_PERMISSION_CONSTANTS_H_

namespace xwalk {
namespace application {

typedef enum {
  ALLOW_ONCE,
  ALLOW_SESSION,
  ALLOW_FOREVER,
  DENY_ONCE,
  DENY_SESSION,
  DENY_FOREVER,
  INVALID_RUNTIME_PERM
} RuntimePermission;

typedef enum {
  ALLOW,
  DENY,
  ASK,
  INVALID_STORED_PERM
} StoredPermission;

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_PERMISSION_CONSTANTS_H_
