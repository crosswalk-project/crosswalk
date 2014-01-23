// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/native_app_window_tizen.h"

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "ui/aura/root_window.h"
#include "ui/base/x/x11_util.h"
#include "ui/gfx/transform.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/screen.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"
#include "xwalk/runtime/browser/ui/top_view_layout_views.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts_tizen.h"

namespace xwalk {

NativeAppWindowTizen::NativeAppWindowTizen(
    const NativeAppWindow::CreateParams& create_params)
    : NativeAppWindowViews(create_params),
      indicator_widget_(new TizenSystemIndicatorWidget()),
      allowed_orientations_(ANY) {
  indicator_container_.reset(new WidgetContainerView(indicator_widget_));
}

void NativeAppWindowTizen::Initialize() {
  NativeAppWindowViews::Initialize();

  // Get display info such as device_scale_factor, and current
  // rotation (orientation). NOTE: This is a local copy of the info.
  display_ = gfx::Screen::GetNativeScreen()->GetPrimaryDisplay();

  aura::Window* root_window = GetNativeWindow()->GetRootWindow();
  DCHECK(root_window);
  root_window->AddObserver(this);

  OrientationMask ua_default =
      XWalkBrowserMainPartsTizen::GetAllowedUAOrientations();
  OnAllowedOrientationsChanged(ua_default);

  if (SensorProvider* sensor = SensorProvider::GetInstance()) {
    gfx::Display::Rotation rotation
      = GetClosestAllowedRotation(sensor->GetCurrentRotation());
    display_.set_rotation(rotation);
    ApplyDisplayRotation();
    sensor->AddObserver(this);
  }
}

NativeAppWindowTizen::~NativeAppWindowTizen() {
  if (SensorProvider::GetInstance())
    SensorProvider::GetInstance()->RemoveObserver(this);
}

void NativeAppWindowTizen::UpdateXWindowPid(base::ProcessId pid) {
  XID xid = GetNativeWindow()->GetDispatcher()->GetAcceleratedWidget();
  base::ProcessId curr_pid;
  if (!ui::GetIntProperty(xid, "_NET_WM_PID", &curr_pid) ||
      curr_pid == pid)
    return;

  ui::SetIntProperty(xid, "_NET_WM_PID", "CARDINAL", pid);
}

void NativeAppWindowTizen::ViewHierarchyChanged(
    const ViewHierarchyChangedDetails& details) {
  if (details.is_add && details.child == this) {
    NativeAppWindowViews::ViewHierarchyChanged(details);
    indicator_widget_->Initialize(GetNativeWindow());
    top_view_layout()->set_top_view(indicator_container_.get());
    AddChildView(indicator_container_.get());
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
  if (visible)
    ApplyDisplayRotation();
}

gfx::Transform NativeAppWindowTizen::GetRotationTransform() const {
  // This method assumed a fixed portrait device. As everything
  // is calculated from the fixed position we do not update the
  // display bounds after rotation change.
  gfx::Transform rotate;
  float one_pixel = 1.0f / display_.device_scale_factor();
  switch (display_.rotation()) {
    case gfx::Display::ROTATE_0:
      break;
    case gfx::Display::ROTATE_90:
      rotate.Translate(display_.bounds().width() - one_pixel, 0);
      rotate.Rotate(90);
      break;
    case gfx::Display::ROTATE_270:
      rotate.Translate(0, display_.bounds().height() - one_pixel);
      rotate.Rotate(270);
      break;
    case gfx::Display::ROTATE_180:
      rotate.Translate(display_.bounds().width() - one_pixel,
                       display_.bounds().height() - one_pixel);
      rotate.Rotate(180);
      break;
  }

  return rotate;
}

namespace {

// Rotates a binary mask of 4 positions to the left.
unsigned rotl4(unsigned value, int shift) {
  unsigned res = (value << shift);
  if (res > (1 << 4) - 1)
    res = res >> 4;
  return res;
}

bool IsLandscapeOrientation(const gfx::Display::Rotation& rotation) {
  return rotation == gfx::Display::ROTATE_90 ||
         rotation == gfx::Display::ROTATE_270;
}

}  // namespace.

gfx::Display::Rotation NativeAppWindowTizen::GetClosestAllowedRotation(
    gfx::Display::Rotation rotation) const {

  unsigned result = PORTRAIT_PRIMARY;
  // gfx::Display::Rotation starts at portrait-primary and
  // belongs to the set [0:3].
  result = rotl4(result, rotation);

  // Test current orientation
  if (allowed_orientations_ & result)
    return rotation;

  // Test orientation right of current one.
  if (allowed_orientations_ & rotl4(result, 1))
    return static_cast<gfx::Display::Rotation>((rotation + 1) % 4);

  // Test orientation left of current one.
  if (allowed_orientations_ & rotl4(result, 3))
    return static_cast<gfx::Display::Rotation>((rotation + 3) % 4);

  // Test orientation opposite of current one.
  if (allowed_orientations_ & rotl4(result, 2))
    return static_cast<gfx::Display::Rotation>((rotation + 2) % 4);

  NOTREACHED();
  return rotation;
}

Orientation NativeAppWindowTizen::GetCurrentOrientation() const {
  switch (display_.rotation()) {
    case gfx::Display::ROTATE_0:
      return PORTRAIT_PRIMARY;
    case gfx::Display::ROTATE_90:
      return LANDSCAPE_PRIMARY;
    case gfx::Display::ROTATE_180:
      return PORTRAIT_SECONDARY;
    case gfx::Display::ROTATE_270:
      return LANDSCAPE_SECONDARY;
    default:
      NOTREACHED();
      return PORTRAIT_PRIMARY;
  }
}

void NativeAppWindowTizen::OnAllowedOrientationsChanged(
    OrientationMask orientations) {
  allowed_orientations_ = orientations;

  // As we might have been locked before our current orientation
  // might not fit with the sensor orienation.
  gfx::Display::Rotation rotation = display_.rotation();
  if (SensorProvider* sensor = SensorProvider::GetInstance())
    rotation = sensor->GetCurrentRotation();

  rotation = GetClosestAllowedRotation(rotation);
  if (display_.rotation() == rotation)
    return;

  display_.set_rotation(rotation);
  ApplyDisplayRotation();
}

void NativeAppWindowTizen::OnRotationChanged(
    gfx::Display::Rotation rotation) {
  // We always store the current sensor position, even if we do not
  // apply it in case the window is invisible.

  rotation = GetClosestAllowedRotation(rotation);
  if (display_.rotation() == rotation)
    return;

  display_.set_rotation(rotation);

  ApplyDisplayRotation();
}

void NativeAppWindowTizen::UpdateTopViewOverlay() {
  top_view_layout()->SetUseOverlay(
      IsLandscapeOrientation(display_.rotation()));
}

void NativeAppWindowTizen::ApplyDisplayRotation() {
  if (observer())
    observer()->OnOrientationChanged(GetCurrentOrientation());

  aura::Window* root_window = GetNativeWindow()->GetRootWindow();
  if (!root_window->IsVisible())
    return;
  UpdateTopViewOverlay();
  indicator_widget_->SetDisplay(display_);
  root_window->SetTransform(GetRotationTransform());
}

}  // namespace xwalk
