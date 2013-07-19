// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_platform_util.h"

namespace platform_util {

gfx::NativeWindow GetTopLevel(gfx::NativeView view) {
  return 0;
}

gfx::NativeView GetParent(gfx::NativeView view) {
  return 0;
}

bool IsWindowActive(gfx::NativeWindow window) {
  return true;
}

void ActivateWindow(gfx::NativeWindow window) {
}

bool IsVisible(gfx::NativeView view) {
  return true;
}

}  // namespace platform_util
