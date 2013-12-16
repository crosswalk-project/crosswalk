// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/native_app_window_tizen.h"

#include "base/memory/scoped_ptr.h"
#include "ui/aura/root_window.h"
#include "ui/gfx/transform.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/screen.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"
#include "xwalk/runtime/browser/ui/top_view_layout_views.h"

namespace xwalk {

NativeAppWindowTizen::NativeAppWindowTizen(
    const NativeAppWindow::CreateParams& create_params)
    : NativeAppWindowViews(create_params),
      indicator_(new TizenSystemIndicator()) {
}

void NativeAppWindowTizen::Initialize() {
  NativeAppWindowViews::Initialize();

  // Get display info such as device_scale_factor, and current
  // rotation (orientation). NOTE: This is a local copy of the info.
  display_ = gfx::Screen::GetNativeScreen()->GetPrimaryDisplay();

  aura::Window* root_window = GetNativeWindow()->GetRootWindow();
  DCHECK(root_window);
  root_window->AddObserver(this);

  if (SensorProvider* sensor = SensorProvider::GetInstance()) {
    OnRotationChanged(sensor->GetCurrentRotation());
    sensor->AddObserver(this);
  }
}

NativeAppWindowTizen::~NativeAppWindowTizen() {
  if (SensorProvider::GetInstance())
    SensorProvider::GetInstance()->RemoveObserver(this);
  aura::Window* root_window = GetNativeWindow()->GetRootWindow();
  DCHECK(root_window);
  root_window->RemoveObserver(this);
}

void NativeAppWindowTizen::ViewHierarchyChanged(
    const ViewHierarchyChangedDetails& details) {
  if (details.is_add && details.child == this) {
    NativeAppWindowViews::ViewHierarchyChanged(details);

    AddChildView(indicator_.get());
    top_view_layout()->set_top_view(indicator_.get());
  }
}

void NativeAppWindowTizen::OnWindowBoundsChanged(
    aura::Window* window,
    const gfx::Rect& old_bounds,
    const gfx::Rect& new_bounds) {
  aura::Window* root_window = GetNativeWindow()->GetRootWindow();
  DCHECK_EQ(root_window, window);

  // We are working with DIPs here. size() returns in DIPs.
  GetWidget()->GetRootView()->SetSize(new_bounds.size());
}

void NativeAppWindowTizen::OnWindowVisibilityChanging(aura::Window* window,
                                                      bool visible) {
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

void NativeAppWindowTizen::ApplyDisplayRotation() {
  aura::Window* root_window = GetNativeWindow()->GetRootWindow();
  root_window->SetTransform(GetRotationTransform());
  indicator_->SetDisplay(display_);
}

void NativeAppWindowTizen::OnRotationChanged(
    gfx::Display::Rotation rotation) {
  // We always store the current sensor position, even if we do not
  // apply it in case the window is invisible.

  // FIXME: Given the current orientation (sensor), set the preferred
  // rotation from the set of allowed orientations for this window.
  display_.set_rotation(rotation);

  aura::Window* root_window = GetNativeWindow()->GetRootWindow();
  if (root_window->IsVisible())
    ApplyDisplayRotation();
}

}  // namespace xwalk
