// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_TIZEN_H_
#define XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_TIZEN_H_

#include "xwalk/runtime/browser/ui/screen_orientation.h"
#include "xwalk/runtime/browser/ui/native_app_window_views.h"
#include "xwalk/tizen/mobile/sensor/sensor_provider.h"
#include "xwalk/tizen/mobile/ui/tizen_system_indicator_widget.h"
#include "xwalk/tizen/mobile/ui/widget_container_view.h"
#include "ui/aura/window_observer.h"

namespace xwalk {

// Tizen uses the Views native window but adds its own features like orientation
// handling and integration with system indicator bar.
class NativeAppWindowTizen
    : public aura::WindowObserver,
      public NativeAppWindowViews,
      public SensorProvider::Observer,
      public MultiOrientationScreen {
 public:
  explicit NativeAppWindowTizen(const NativeAppWindow::CreateParams& params);
  virtual ~NativeAppWindowTizen();


 private:
  gfx::Transform GetRotationTransform() const;
  void ApplyDisplayRotation();

  // NativeAppWindowViews overrides:
  virtual void Initialize() OVERRIDE;

  // WindowObserver overrides:
  virtual void OnWindowVisibilityChanging(aura::Window* window,
                                          bool visible) OVERRIDE;
  virtual void OnWindowBoundsChanged(aura::Window* window,
                                     const gfx::Rect& old_bounds,
                                     const gfx::Rect& new_bounds) OVERRIDE;
  virtual void OnWindowDestroying(aura::Window* window) OVERRIDE;

  // views::View overrides:
  virtual void ViewHierarchyChanged(
      const ViewHierarchyChangedDetails& details) OVERRIDE;

  // SensorProvider::Observer overrides:
  virtual void OnRotationChanged(gfx::Display::Rotation rotation) OVERRIDE;

  void UpdateTopViewOverlay();

  // MultiOrientationScreen overrides:
  virtual Orientation GetCurrentOrientation() const OVERRIDE;
  virtual void OnAllowedOrientationsChanged(
      OrientationMask orientations) OVERRIDE;

  gfx::Display::Rotation GetClosestAllowedRotation(
      gfx::Display::Rotation) const;

  // The system indicator is implemented as a widget because it needs to
  // receive events and may also be an overlay on top of the rest of the
  // content, regular views do not support this. We add it to the container,
  // that is a view that doesn't draw anything, just passes the bounds
  // information to the topview layout.
  scoped_ptr<TizenSystemIndicatorWidget> indicator_widget_;
  scoped_ptr<WidgetContainerView> indicator_container_;
  gfx::Display display_;
  OrientationMask allowed_orientations_;

  DISALLOW_COPY_AND_ASSIGN(NativeAppWindowTizen);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_TIZEN_H_
