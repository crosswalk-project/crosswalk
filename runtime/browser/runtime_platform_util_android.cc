// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_platform_util.h"

#include "ui/base/android/view_android.h"

namespace platform_util {

gfx::NativeWindow GetTopLevel(gfx::NativeView view) {
  return view->GetWindowAndroid();
}

gfx::NativeView GetParent(gfx::NativeView view) {
  return view;
}

bool IsWindowActive(gfx::NativeWindow window) {
  NOTIMPLEMENTED();
  return false;
}

void ActivateWindow(gfx::NativeWindow window) {
  NOTIMPLEMENTED();
}

bool IsVisible(gfx::NativeView view) {
  NOTIMPLEMENTED();
  return true;
}

}  // namespace platform_util
