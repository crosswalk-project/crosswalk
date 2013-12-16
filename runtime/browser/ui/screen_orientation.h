// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_SCREEN_ORIENTATION_H_
#define XWALK_RUNTIME_BROWSER_UI_SCREEN_ORIENTATION_H_

#include <set>

#include "ui/gfx/display.h"

namespace xwalk {

class ScreenOrientation {
 public:
  enum Mode {
    PORTRAIT_PRIMARY    = 0x1,
    PORTRAIT_SECONDARY  = 0x2,
    PORTRAIT            = 0x3,
    LANDSCAPE_PRIMARY   = 0x4,
    LANDSCAPE_SECONDARY = 0x8,
    LANDSCAPE           = 0xc,
    ANY_ORIENTATION     = 0xf,
  };

  class Observer {
   public:
    virtual void OnScreenOrientationChanged(Mode mode) = 0;
  };

  ScreenOrientation();
  virtual ~ScreenOrientation();

  virtual void AddObserver(Observer* observer);
  virtual void RemoveObserver(Observer* observer);

  virtual void LockOrientation(Mode modes);
  virtual void UnlockOrientation() {
    LockOrientation(ANY_ORIENTATION);
  }

  virtual Mode GetOrientation() const {
    return current_orientation_;
  }
  virtual void SetRotation(gfx::Display::Rotation rotation);
  static gfx::Display::Rotation ToRotation(Mode mode);

 protected:
  virtual bool IsAllowed(Mode mode) const {
    return mode & allowed_orientations_;
  }
  virtual Mode ElectOrientation() const;
  virtual void ChangeOrientation(Mode mode);

  Mode allowed_orientations_;
  Mode current_orientation_;
  Mode real_orientation_;

  std::set<Observer*> observers_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_SCREEN_ORIENTATION_H_
