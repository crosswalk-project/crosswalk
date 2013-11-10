// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_ENTRYPOINTS_H_
#define XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_ENTRYPOINTS_H_

// NOTE: This file and interfaces marked as internal are not considered stable
// and can be modified in incompatible ways between Crosswalk versions.

#ifndef XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_H_
#error "You should include XW_Extension.h before this file"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define XW_INTERNAL_ENTRY_POINTS_INTERFACE_1 \
  "XW_Internal_EntryPointsInterface_1"
#define XW_INTERNAL_ENTRY_POINTS_INTERFACE \
  XW_INTERNAL_ENTRY_POINTS_INTERFACE_1

//
// XW_INTERNAL_ENTRY_POINTS_INTERFACE: provides a way for extensions to add
// more information about its implementation. For now, allow extensions to
// specify more objects that the access should cause the extension to be
// loaded.
//

struct XW_Internal_EntryPointsInterface_1 {
  // Register extra entry points for this extension. An "extra" entry points
  // are objects outside the implicit namespace for which the extension should
  // be loaded when they are touched.
  //
  // This function should be called only during XW_Initialize().
  void (*SetExtraJSEntryPoints)(XW_Extension extension,
                                const char** entry_points);
};

typedef struct XW_Internal_EntryPointsInterface_1
    XW_Internal_EntryPointsInterface;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_ENTRYPOINTS_H_

