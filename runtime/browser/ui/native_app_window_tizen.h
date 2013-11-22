// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_TIZEN_H_
#define XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_TIZEN_H_

#include "xwalk/runtime/browser/ui/native_app_window_views.h"
#include "xwalk/runtime/browser/tizen/sensor_provider.h"

namespace xwalk {

// Tizen uses the Views native window but adds its own features like orientation
// handling and integration with system indicator bar.
class NativeAppWindowTizen : public NativeAppWindowViews,
                             public SensorProvider::Observer {
 public:
  explicit NativeAppWindowTizen(const NativeAppWindow::CreateParams& params);
  virtual ~NativeAppWindowTizen();

 private:
  // views::View implementation.
  virtual void ViewHierarchyChanged(
      const ViewHierarchyChangedDetails& details) OVERRIDE;

  // SensorProvider::Observer implementation.
  virtual void OnRotationChanged(gfx::Display::Rotation rotation) OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(NativeAppWindowTizen);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_TIZEN_H_
