// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_TIZEN_XWINDOW_PROVIDER_EFL_H_
#define XWALK_RUNTIME_BROWSER_UI_TIZEN_XWINDOW_PROVIDER_EFL_H_

#include <X11/Xlib.h>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "ui/gfx/rect.h"
#include "xwalk/runtime/browser/ui/tizen/xwindow_provider_delegate_efl.h"

namespace views {

class XWindowProvider {
 public:
  XWindowProvider(XWindowProviderDelegate* delegate, gfx::Rect);
  ~XWindowProvider();

  static gfx::Rect GetWindowGeometry();

  ::Window GetXWindow() const;
  gfx::Rect GetBounds() const;

  void Move(int x, int y);
  void Resize(int w, int h);
  void Show();
  void Hide();

 private:
  struct Private;
  scoped_ptr<Private> private_;

  DISALLOW_COPY_AND_ASSIGN(XWindowProvider);
};

}  // namespace views

#endif  // XWALK_RUNTIME_BROWSER_UI_TIZEN_XWINDOW_PROVIDER_EFL_H_
