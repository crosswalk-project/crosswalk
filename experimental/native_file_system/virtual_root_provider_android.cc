// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/native_file_system/virtual_root_provider.h"

#include "xwalk/runtime/browser/android/xwalk_path_helper.h"

VirtualRootProvider::VirtualRootProvider() {
  virtual_root_map_ = xwalk::XWalkPathHelper::GetVirtualRootMap();
}
