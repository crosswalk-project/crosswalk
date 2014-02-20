// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_PERMISSIONS_H_
#define XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_PERMISSIONS_H_

// NOTE: This file and interfaces marked as internal are not considered stable
// and can be modified in incompatible ways between Crosswalk versions.

#ifndef XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_H_
#error "You should include XW_Extension.h before this file"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define XW_INTERNAL_PERMISSIONS_INTERFACE_1 \
    "XW_Internal_PermissionsInterface_1"
#define XW_INTERNAL_PERMISSIONS_INTERFACE \
    XW_INTERNAL_PERMISSIONS_INTERFACE_1

//
// XW_INTERNAL_PERMISSIONS_INTERFACE: provides a way for extensions
// check if they have the proper permissions for certain APIs.
//

struct XW_Internal_PermissionsInterface_1 {
  int (*CheckAPIAccessControl)(XW_Extension extension, const char* api_name);
  int (*RegisterPermissions)(XW_Extension extension, const char* perm_table);
};

typedef struct XW_Internal_PermissionsInterface_1
    XW_Internal_PermissionsInterface;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_PERMISSIONS_H_
