// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/native_app_window_tizen.h"

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "ui/aura/window.h"
#include "ui/gfx/transform.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/screen.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"
#include "xwalk/runtime/browser/ui/top_view_layout_views.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts_tizen.h"

namespace {

static gfx::Display::Rotation ToDisplayRotation(gfx::Display display,
    blink::WebScreenOrientationType orientation) {
  gfx::Display::Rotation rot = gfx::Display::ROTATE_0;
  switch (orientation) {
    case blink::WebScreenOrientationUndefined:
    case blink::WebScreenOrientationPortraitPrimary:
      rot = gfx::Display::ROTATE_0;
      break;
    case blink::WebScreenOrientationLandscapeSecondary:
      rot = gfx::Display::ROTATE_90;
      break;
    case blink::WebScreenOrientationPortraitSecondary:
      rot = gfx::Display::ROTATE_180;
      break;
    case blink::WebScreenOrientationLandscapePrimary:
      rot = gfx::Display::ROTATE_270;
      break;
  default:
      NOTREACHED();
  }

  if (display.bounds().width() > display.bounds().height()) {
    // Landscape devices have landscape-primary as default.
    rot = static_cast<gfx::Display::Rotation>((rot - 1) % 4);
  }

  return rot;
}

static void SetWindowRotation(aura::Window* window, gfx::Display display) {
  // This methods assumes that window is fullscreen.

#if defined(OS_TIZEN_MOBILE)
  // Assumes portrait display; shows overlay indicator in landscape only.
  bool useOverlay = display.rotation() == gfx::Display::ROTATE_90 ||
      display.rotation() == gfx::Display::ROTATE_180);
  top_view_layout()->SetUseOverlay(enableOverlay);
  indicator_widget_->SetDisplay(display);
#endif

  // As everything is calculated from the fixed position we do
  // not update the display bounds after rotation change.
  gfx::Transform rotate;
  float one_pixel = 1.0f / display.device_scale_factor();
  switch (display.rotation()) {
    case gfx::Display::ROTATE_0:
      break;
    case gfx::Display::ROTATE_90:
      rotate.Translate(display.bounds().width() - one_pixel, 0);
      rotate.Rotate(90);
      break;
    case gfx::Display::ROTATE_270:
      rotate.Translate(0, display.bounds().height() - one_pixel);
      rotate.Rotate(270);
      break;
    case gfx::Display::ROTATE_180:
      rotate.Translate(display.bounds().width() - one_pixel,
                       display.bounds().height() - one_pixel);
      rotate.Rotate(180);
      break;
  }

  window->SetTransform(rotate);
}

}  // namespace.

namespace xwalk {

NativeAppWindowTizen::NativeAppWindowTizen(
    const NativeAppWindow::CreateParams& create_params)
    : NativeAppWindowViews(create_params),
#if defined(OS_TIZEN_MOBILE)
      indicator_widget_(new TizenSystemIndicatorWidget()),
      indicator_container_(new WidgetContainerView(indicator_widget_)),
#endif
      orientation_lock_(blink::WebScreenOrientationLockAny) {
}

void NativeAppWindowTizen::Initialize() {
  NativeAppWindowViews::Initialize();

  // Get display info such as device_scale_factor, and current
  // rotation (orientation).
  // NOTE: This is a local copy of the info.
  display_ = gfx::Screen::GetNativeScreen()->GetPrimaryDisplay();

  aura::Window* root_window = GetNativeWindow()->GetRootWindow();
  DCHECK(root_window);
  root_window->AddObserver(this);

  if (SensorProvider* sensor = SensorProvider::GetInstance()) {
    sensor->AddObserver(this);
    OnScreenOrientationChanged(sensor->GetScreenOrientation());
  }
}

NativeAppWindowTizen::~NativeAppWindowTizen() {
  if (SensorProvider::GetInstance())
    SensorProvider::GetInstance()->RemoveObserver(this);
}

void NativeAppWindowTizen::ViewHierarchyChanged(
    const ViewHierarchyChangedDetails& details) {
  if (details.is_add && details.child == this) {
    NativeAppWindowViews::ViewHierarchyChanged(details);

#if defined(OS_TIZEN_MOBILE)
    indicator_widget_->Initialize(GetNativeWindow());
    top_view_layout()->set_top_view(indicator_container_.get());
    AddChildView(indicator_container_.get());
#endif
  }
}

void NativeAppWindowTizen::OnWindowBoundsChanged(
    aura::Window* window,
    const gfx::Rect& old_bounds,
    const gfx::Rect& new_bounds) {
  aura::Window* root_window = GetNativeWindow()->GetRootWindow();
  DCHECK_EQ(root_window, window);

  // Change the bounds of child windows to make touch work correctly.
  GetNativeWindow()->parent()->SetBounds(new_bounds);
  GetNativeWindow()->SetBounds(new_bounds);

  GetWidget()->GetRootView()->SetSize(new_bounds.size());
}

void NativeAppWindowTizen::OnWindowDestroying(aura::Window* window) {
  // Must be removed here and not in the destructor, as the aura::Window is
  // already destroyed when our destructor runs.
  window->RemoveObserver(this);
}

void NativeAppWindowTizen::OnWindowVisibilityChanging(
    aura::Window* window, bool visible) {
  if (!visible)
    return;
  SetDisplayRotation(display_);
}

blink::WebScreenOrientationType
    NativeAppWindowTizen::FindNearestAllowedOrientation(
        blink::WebScreenOrientationType orientation) const {
  switch (orientation_lock_) {
    case blink::WebScreenOrientationLockDefault:
    case blink::WebScreenOrientationLockAny:
      return orientation;
    case blink::WebScreenOrientationLockLandscape: {
      switch (orientation) {
        case blink::WebScreenOrientationLandscapePrimary:
        case blink::WebScreenOrientationLandscapeSecondary:
          return orientation;
        default:
          return blink::WebScreenOrientationLandscapePrimary;
      }
      break;
    }
    case blink::WebScreenOrientationLockPortrait: {
      switch (orientation) {
        case blink::WebScreenOrientationPortraitPrimary:
        case blink::WebScreenOrientationPortraitSecondary:
          return orientation;
        default:
          return blink::WebScreenOrientationPortraitPrimary;
      }
      break;
    }
    case blink::WebScreenOrientationLockPortraitPrimary:
      return blink::WebScreenOrientationPortraitPrimary;
    case blink::WebScreenOrientationLockPortraitSecondary:
      return blink::WebScreenOrientationPortraitSecondary;
    case blink::WebScreenOrientationLockLandscapePrimary:
      return blink::WebScreenOrientationLandscapePrimary;
    case blink::WebScreenOrientationLockLandscapeSecondary:
      return blink::WebScreenOrientationLandscapeSecondary;
  default:
      NOTREACHED();
  }
  return orientation;
}

void NativeAppWindowTizen::LockOrientation(
      blink::WebScreenOrientationLockType lock) {
  orientation_lock_ = lock;
  if (SensorProvider* sensor = SensorProvider::GetInstance())
    OnScreenOrientationChanged(sensor->GetScreenOrientation());
}

void NativeAppWindowTizen::UnlockOrientation() {
  LockOrientation(blink::WebScreenOrientationLockDefault);
}

void NativeAppWindowTizen::OnScreenOrientationChanged(
    blink::WebScreenOrientationType orientation) {

  // We always store the current sensor position, even if we do not
  // apply it in case the window is invisible.
  gfx::Display::Rotation rot = ToDisplayRotation(display_,
      FindNearestAllowedOrientation(orientation));
  if (display_.rotation() == rot)
    return;

  display_.set_rotation(rot);
  SetDisplayRotation(display_);
}

void NativeAppWindowTizen::SetDisplayRotation(gfx::Display display) {
  aura::Window* window = GetNativeWindow()->GetRootWindow();
  if (!window->IsVisible())
    return;

  SetWindowRotation(window, display);
}

}  // namespace xwalk
