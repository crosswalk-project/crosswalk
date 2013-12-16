// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/screen_orientation.h"

#include "base/logging.h"

namespace xwalk {

ScreenOrientation::ScreenOrientation()
    : allowed_orientations_(ANY_ORIENTATION),
      current_orientation_(PORTRAIT_PRIMARY),
      real_orientation_(PORTRAIT_PRIMARY) {
}

ScreenOrientation::~ScreenOrientation() {
}

void ScreenOrientation::LockOrientation(Mode modes) {
  CHECK(modes);
  allowed_orientations_ = modes;

  if (IsAllowed(real_orientation_)) {
    // Change to the real device orientation,
    // if it was disabled but allowed now.
    ChangeOrientation(real_orientation_);
  } else if (!IsAllowed(current_orientation_)) {
    // Change to a locked orientation,
    // if the current orientation is disabled.
    ChangeOrientation(ElectOrientation());
  }
}

void ScreenOrientation::SetRotation(gfx::Display::Rotation rotation) {
  switch (rotation) {
    case gfx::Display::ROTATE_0:
      real_orientation_ = PORTRAIT_PRIMARY;
      break;
    case gfx::Display::ROTATE_90:
      real_orientation_ = LANDSCAPE_PRIMARY;
      break;
    case gfx::Display::ROTATE_180:
      real_orientation_ = PORTRAIT_SECONDARY;
      break;
    case gfx::Display::ROTATE_270:
      real_orientation_ = LANDSCAPE_SECONDARY;
      break;
    default:
      NOTREACHED();
  }

  if (IsAllowed(real_orientation_))
    ChangeOrientation(real_orientation_);
}

gfx::Display::Rotation ScreenOrientation::ToRotation(Mode mode) {
  switch (mode) {
    case PORTRAIT_PRIMARY:
      return gfx::Display::ROTATE_0;
    case PORTRAIT_SECONDARY:
      return gfx::Display::ROTATE_180;
    case LANDSCAPE_PRIMARY:
      return gfx::Display::ROTATE_90;
    case LANDSCAPE_SECONDARY:
      return gfx::Display::ROTATE_270;
    default:
      NOTREACHED();
  }
  return gfx::Display::ROTATE_0;
}

void ScreenOrientation::AddObserver(Observer* observer) {
  observers_.insert(observer);
}

void ScreenOrientation::RemoveObserver(Observer* observer) {
  observers_.erase(observer);
}

ScreenOrientation::Mode ScreenOrientation::ElectOrientation() const {
  static Mode modes[] = {
    PORTRAIT_PRIMARY, PORTRAIT_SECONDARY,
    LANDSCAPE_PRIMARY, LANDSCAPE_SECONDARY
  };

  for (size_t i = 0; i < sizeof(modes) / sizeof(Mode); ++i)
    if (IsAllowed(modes[i]))
      return modes[i];

  NOTREACHED();
  return PORTRAIT_PRIMARY;
}

void ScreenOrientation::ChangeOrientation(Mode mode) {
  if (mode == current_orientation_)
    return;
  current_orientation_ = mode;

  std::set<Observer*>::iterator it;
  for (it = observers_.begin(); it != observers_.end(); ++it)
    (*it)->OnScreenOrientationChanged(mode);
}

}  // namespace xwalk
