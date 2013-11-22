// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/security/permission_group_internal.h"

namespace xwalk {
namespace application {

// TODO(Bai): Complete this class implementation according to the
// permission list:
// https://docs.google.com/a/intel.com/spreadsheet/ccc?key=
// 0AmfuGardsG7gdGg1a0YxVVVNbEtKLTEzck9XMGYyRWc#gid=0
PermissionGroupInternal::PermissionGroupInternal() {
}

PermissionGroupInternal::~PermissionGroupInternal() {
}

bool PermissionGroupInternal::GetPermissionName(
    const std::string& method_name,
    std::string& perm_name) {
  return false;
}

bool PermissionGroupInternal::IsValidPermissionName(
    const std::string& perm_name) {
  return false;
}

}  // namespace application
}  // namespace xwalk
