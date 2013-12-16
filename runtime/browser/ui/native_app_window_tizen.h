// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_TIZEN_H_
#define XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_TIZEN_H_

#include "xwalk/runtime/browser/ui/native_app_window_views.h"
#include "xwalk/runtime/browser/ui/screen_orientation.h"
#include "xwalk/tizen/mobile/sensor/sensor_provider.h"
#include "xwalk/tizen/mobile/ui/tizen_system_indicator.h"
#include "ui/aura/window_observer.h"

namespace xwalk {

// Tizen uses the Views native window but adds its own features like orientation
// handling and integration with system indicator bar.
class NativeAppWindowTizen : public aura::WindowObserver,
                             public NativeAppWindowViews,
                             public ScreenOrientation,
                             public ScreenOrientation::Observer,
                             public SensorProvider::Observer {
 public:
  explicit NativeAppWindowTizen(const NativeAppWindow::CreateParams& params);
  virtual ~NativeAppWindowTizen();

 private:
  // NativeAppWindowViews overrides:
  virtual void Initialize() OVERRIDE;

  // WindowObserver overrides:
  virtual void OnWindowVisibilityChanging(aura::Window* window,
                                          bool visible) OVERRIDE;
  virtual void OnWindowBoundsChanged(aura::Window* window,
                                     const gfx::Rect& old_bounds,
                                     const gfx::Rect& new_bounds) OVERRIDE;

  // views::View overrides:
  virtual void ViewHierarchyChanged(
      const ViewHierarchyChangedDetails& details) OVERRIDE;

  gfx::Transform GetRotationTransform() const;
  void ApplyDisplayRotation();

  // ScreenOrientation::Observer overrides:
  virtual void OnScreenOrientationChanged(
      ScreenOrientation::Mode mode) OVERRIDE;

  // SensorProvider::Observer overrides:
  virtual void OnRotationChanged(gfx::Display::Rotation rotation) OVERRIDE;

  scoped_ptr<TizenSystemIndicator> indicator_;
  gfx::Display display_;

  DISALLOW_COPY_AND_ASSIGN(NativeAppWindowTizen);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_TIZEN_H_
