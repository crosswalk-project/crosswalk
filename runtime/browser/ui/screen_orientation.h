// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_SCREEN_ORIENTATION_H_
#define XWALK_RUNTIME_BROWSER_UI_SCREEN_ORIENTATION_H_

namespace xwalk {

enum Orientation {
// These are ordered as they appear clockwise.
PORTRAIT_PRIMARY    = 1 << 0,
LANDSCAPE_PRIMARY   = 1 << 1,
PORTRAIT_SECONDARY  = 1 << 2,
LANDSCAPE_SECONDARY = 1 << 3,

// Combinations
PORTRAIT            = PORTRAIT_PRIMARY | PORTRAIT_SECONDARY,
LANDSCAPE           = LANDSCAPE_PRIMARY | LANDSCAPE_SECONDARY,
ANY                 = PORTRAIT | LANDSCAPE
};

typedef unsigned OrientationMask;

class ScreenOrientationAPISupplement {
 public:
  virtual ~ScreenOrientationAPISupplement() {}
  virtual OrientationMask GetAllowedUAOrientations() const = 0;
  virtual void OnAllowedOrientationsChanged(OrientationMask orientations) = 0;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_SCREEN_ORIENTATION_H_
