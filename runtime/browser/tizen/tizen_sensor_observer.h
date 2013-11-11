// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_TIZEN_TIZEN_SENSOR_OBSERVER_H_
#define XWALK_RUNTIME_BROWSER_TIZEN_TIZEN_SENSOR_OBSERVER_H_

#include "ui/gfx/transform.h"

#include "xwalk/runtime/browser/tizen/sensor_provider.h"

namespace xwalk {

class NativeAppWindowViews;

class TizenSensorObserver : public SensorProvider::Observer {
 public:
  explicit TizenSensorObserver(NativeAppWindowViews* window);
  virtual ~TizenSensorObserver();

 private:
  virtual void OnRotationChanged(gfx::Display::Rotation rotation) OVERRIDE;

  gfx::Transform GetTransform(gfx::Display::Rotation rotation) const;

  NativeAppWindowViews* window_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(TizenSensorObserver);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_TIZEN_TIZEN_SENSOR_OBSERVER_H_
