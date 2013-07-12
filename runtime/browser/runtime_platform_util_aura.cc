// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/platform_util.h"

#include "base/logging.h"
#include "ui/aura/window.h"

#if defined(USE_ASH)
#include "ash/wm/window_util.h"
#endif

namespace platform_util {

gfx::NativeWindow GetTopLevel(gfx::NativeView view) {
  return view->GetToplevelWindow();
}

gfx::NativeView GetParent(gfx::NativeView view) {
  return view->parent();
}

bool IsWindowActive(gfx::NativeWindow window) {
#if defined(USE_ASH)
  return ash::wm::IsActiveWindow(window);
#else
  NOTIMPLEMENTED();
  return false;
#endif
}

void ActivateWindow(gfx::NativeWindow window) {
#if defined(USE_ASH)
  ash::wm::ActivateWindow(window);
#else
  NOTIMPLEMENTED();
#endif
}

bool IsVisible(gfx::NativeView view) {
  return view->IsVisible();
}

}  // namespace platform_util