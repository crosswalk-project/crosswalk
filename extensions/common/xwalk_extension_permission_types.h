// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_PERMISSION_TYPES_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_PERMISSION_TYPES_H_

namespace xwalk {
namespace extensions {

// TODO(Bai): For the moment, the enum value must be strictly aligned with
// what is defined in application/common/permission_types.h
// Move the header into some common place where both application and extension
// can access.
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

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_PERMISSION_TYPES_H_
