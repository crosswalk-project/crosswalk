// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/tizen/tizen_sensor_observer.h"

#include "ui/aura/root_window.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

#include "xwalk/runtime/browser/ui/native_app_window_views.h"

namespace xwalk {

TizenSensorObserver::TizenSensorObserver(NativeAppWindowViews* window)
    : window_(window) {
  if (SensorProvider::GetInstance())
    SensorProvider::GetInstance()->AddObserver(this);
}

TizenSensorObserver::~TizenSensorObserver() {
  if (SensorProvider::GetInstance())
    SensorProvider::GetInstance()->RemoveObserver(this);
}

gfx::Transform TizenSensorObserver::GetTransform(
    gfx::Display::Rotation rotation) const {
  gfx::Size host_size = window_->GetBounds().size();
  gfx::Transform transform;
  switch (rotation) {
    case gfx::Display::ROTATE_0:
      break;
    case gfx::Display::ROTATE_90:
      transform.Translate(host_size.width() - 1, 0);
      transform.Rotate(90);
      break;
    case gfx::Display::ROTATE_180:
      transform.Translate(host_size.width() - 1, host_size.height() - 1);
      transform.Rotate(180);
      break;
    case gfx::Display::ROTATE_270:
      transform.Translate(0, host_size.height() - 1);
      transform.Rotate(270);
      break;
    default:
      NOTREACHED();
  }
  return transform;
}

void TizenSensorObserver::OnRotationChanged(
    gfx::Display::Rotation rotation) {
  // Set transform for root window
  aura::Window* root = window_->GetNativeWindow()->GetRootWindow();
  if (!root)
    return;
  root->SetTransform(GetTransform(rotation));

  // Change the size of root view
  gfx::Size host_size = window_->GetBounds().size();
  gfx::Size size;
  if (rotation == gfx::Display::ROTATE_90 ||
      rotation == gfx::Display::ROTATE_270) {
    size = gfx::Size(host_size.height(), host_size.width());
  } else {
    size = gfx::Size(host_size.width(), host_size.height());
  }
  window_->GetWidget()->GetRootView()->SetSize(size);
}

}  // namespace xwalk
