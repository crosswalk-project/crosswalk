// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_RUNTIME_BROWSER_UI_DESKTOP_EXCLUSIVE_ACCESS_BUBBLE_VIEWS_CONTEXT_H_
#define XWALK_RUNTIME_BROWSER_UI_DESKTOP_EXCLUSIVE_ACCESS_BUBBLE_VIEWS_CONTEXT_H_

#include "ui/gfx/geometry/rect.h"

namespace views {
class Widget;
}

namespace xwalk {
class NativeAppWindow;

// Context in which the exclusive access bubble view is initiated.
class ExclusiveAccessBubbleViewsContext {
 public:
  virtual ~ExclusiveAccessBubbleViewsContext() {}

  // Returns NativeAppWindow.
  virtual xwalk::NativeAppWindow* GetNativeAppViews() = 0;

  // Returns the Widget that hosts the view containing the exclusive access
  // bubble.
  virtual views::Widget* GetBubbleAssociatedWidget() = 0;

  // Returns true if immersive mode is enabled.
  virtual bool IsImmersiveModeEnabled() = 0;

  // Returns the bounds of the top level View in screen coordinate system.
  virtual gfx::Rect GetTopContainerBoundsInScreen() = 0;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_DESKTOP_EXCLUSIVE_ACCESS_BUBBLE_VIEWS_CONTEXT_H_
