// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/platform_util.h"

#include "base/logging.h"
#include "ui/aura/window.h"

namespace platform_util {

gfx::NativeWindow GetTopLevel(gfx::NativeView view) {
  return view->GetToplevelWindow();
}

gfx::NativeView GetParent(gfx::NativeView view) {
  return view->parent();
}

bool IsWindowActive(gfx::NativeWindow window) {
  NOTIMPLEMENTED();
  return false;
}

void ActivateWindow(gfx::NativeWindow window) {
  NOTIMPLEMENTED();
}

bool IsVisible(gfx::NativeView view) {
  return view->IsVisible();
}

}  // namespace platform_util
