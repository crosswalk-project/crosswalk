// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_RUNTIME_H_
#define XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_RUNTIME_H_

// NOTE: This file and interfaces marked as internal are not considered stable
// and can be modified in incompatible ways between Crosswalk versions.

#ifndef XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_H_
#error "You should include XW_Extension.h before this file"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define XW_INTERNAL_RUNTIME_INTERFACE_1 \
  "XW_Internal_RuntimeInterface_1"
#define XW_INTERNAL_RUNTIME_INTERFACE \
  XW_INTERNAL_RUNTIME_INTERFACE_1

//
// XW_INTERNAL_RUNTIME_INTERFACE: allow extensions to gather information
// from the runtime.
//

struct XW_Internal_RuntimeInterface_1 {
  void (*GetRuntimeVariableString)(XW_Extension extension,
                                   const char* key,
                                   char* value,
                                   size_t value_len);
};

typedef struct XW_Internal_RuntimeInterface_1
    XW_Internal_RuntimeInterface;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_RUNTIME_H_

