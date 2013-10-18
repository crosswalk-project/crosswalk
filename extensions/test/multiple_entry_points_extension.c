// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(__cplusplus)
#error "This file is written in C to make sure the C API works as intended."
#endif

#include <stdio.h>
#include <stdlib.h>
#include "xwalk/extensions/public/XW_Extension.h"
#include "xwalk/extensions/public/XW_Extension_EntryPoints.h"
#include "xwalk/extensions/public/XW_Extension_SyncMessage.h"

XW_Extension g_extension = 0;
const XW_CoreInterface* g_core = NULL;
const XW_Internal_EntryPointsInterface* g_entry_points = NULL;

int32_t XW_Initialize(XW_Extension extension, XW_GetInterface get_interface) {
  static const char* kAPI =
      "window.should_exist = true;"
      "exports.also_should_exist = true;";
  static const char* entry_points[] = { "should_exist", NULL };

  g_extension = extension;
  g_core = get_interface(XW_CORE_INTERFACE);
  g_core->SetExtensionName(extension, "xwalk.sample");
  g_core->SetJavaScriptAPI(extension, kAPI);

  g_entry_points = get_interface(XW_INTERNAL_ENTRY_POINTS_INTERFACE);
  g_entry_points->SetExtraJSEntryPoints(extension, entry_points);
  return XW_OK;
}
