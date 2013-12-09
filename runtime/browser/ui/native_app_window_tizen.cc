// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/native_app_window_tizen.h"

#include "base/memory/scoped_ptr.h"
#include "ui/aura/root_window.h"
#include "ui/gfx/transform.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"
#include "xwalk/runtime/browser/ui/top_view_layout_views.h"

namespace xwalk {

NativeAppWindowTizen::NativeAppWindowTizen(
    const NativeAppWindow::CreateParams& create_params)
    : NativeAppWindowViews(create_params),
      indicator_(new TizenSystemIndicator()),
      orientation_(PORTRAIT) {
  if (SensorProvider::GetInstance())
    SensorProvider::GetInstance()->AddObserver(this);
}

NativeAppWindowTizen::~NativeAppWindowTizen() {
  if (SensorProvider::GetInstance())
    SensorProvider::GetInstance()->RemoveObserver(this);
}

void NativeAppWindowTizen::ViewHierarchyChanged(
    const ViewHierarchyChangedDetails& details) {
  if (details.is_add && details.child == this) {
    NativeAppWindowViews::ViewHierarchyChanged(details);
    if (indicator_->IsConnected()) {
      AddChildView(indicator_.get());
      top_view_layout()->set_top_view(indicator_.get());
    } else {
      indicator_.reset();
    }
  }
}

namespace {

gfx::Transform GetRotationAroundCenter(const gfx::Size& size,
                                       const gfx::Display::Rotation& rotation) {
  gfx::Transform transform;
  switch (rotation) {
    case gfx::Display::ROTATE_0:
      break;
    case gfx::Display::ROTATE_90:
      transform.Translate(size.width() - 1, 0);
      transform.Rotate(90);
      break;
    case gfx::Display::ROTATE_180:
      transform.Translate(size.width() - 1, size.height() - 1);
      transform.Rotate(180);
      break;
    case gfx::Display::ROTATE_270:
      transform.Translate(0, size.height() - 1);
      transform.Rotate(270);
      break;
    default:
      NOTREACHED();
  }
  return transform;
}

TizenSystemIndicator::Orientation ConvertToIndicatorOrientation(
  NativeAppWindowTizen::Orientation orientation) {
  switch (orientation) {
     case NativeAppWindowTizen::PORTRAIT:
       return TizenSystemIndicator::PORTRAIT;
     case NativeAppWindowTizen::LANDSCAPE:
       return TizenSystemIndicator::LANDSCAPE;
  }
}

}  // namespace

void NativeAppWindowTizen::SetOrientation(Orientation orientation) {
  if (orientation_ == orientation)
    return;
  orientation_ = orientation;
  if (indicator_) {
    indicator_->SetOrientation(ConvertToIndicatorOrientation(orientation_));
  }
}

void NativeAppWindowTizen::OnRotationChanged(gfx::Display::Rotation rotation) {
  aura::Window* root = GetNativeWindow()->GetRootWindow();
  if (!root)
    return;

  // Set rotation transform for root window. The size of the root window
  // will be changed automaticlly while the transform is set.
  gfx::Rect bounds = GetBounds();
  root->SetTransform(GetRotationAroundCenter(bounds.size(), rotation));

  // Adjust the size of sub-windows
  // FIXME(zliang7): It should follow the change while the root is resized.
  if (rotation == gfx::Display::ROTATE_90 ||
      rotation == gfx::Display::ROTATE_270) {
    int width = bounds.width();
    bounds.set_width(bounds.height());
    bounds.set_height(width);
    SetOrientation(LANDSCAPE);
  } else {
    SetOrientation(PORTRAIT);
  }
  GetNativeWindow()->parent()->SetBounds(bounds);
  GetNativeWindow()->SetBounds(bounds);
  // FIXME(zliang7): Why resizing the widget doesn't work?
  GetWidget()->GetRootView()->SetSize(bounds.size());
}

}  // namespace xwalk
