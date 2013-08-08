// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_TIZEN_PRESERVE_WINDOW_EFL_H_
#define XWALK_RUNTIME_BROWSER_UI_TIZEN_PRESERVE_WINDOW_EFL_H_

#include <Evas.h>
#include <X11/Xlib.h>

#include "base/basictypes.h"
#include "ui/gfx/rect.h"

namespace views {

class PreserveWindow {
 public:
  explicit PreserveWindow(Evas* evas);
  virtual ~PreserveWindow();

  Evas_Object* SmartObject() const { return smart_object_; }
  ::Window GetXWindow() const;

 private:
  Evas_Object* smart_object_;

  DISALLOW_COPY_AND_ASSIGN(PreserveWindow);
};

}  // namespace views

#endif  // XWALK_RUNTIME_BROWSER_UI_TIZEN_PRESERVE_WINDOW_EFL_H_
