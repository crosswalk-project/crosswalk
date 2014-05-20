// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_MOBILE_SENSOR_SENSOR_PROVIDER_H_
#define XWALK_TIZEN_MOBILE_SENSOR_SENSOR_PROVIDER_H_

#include <set>

#include "base/memory/scoped_ptr.h"
#include "ui/gfx/display.h"
#include "third_party/WebKit/public/platform/WebScreenOrientationType.h"

namespace xwalk {

class SensorProvider {
 public:
  static SensorProvider* GetInstance();
  virtual ~SensorProvider();

  class Observer {
   public:
    virtual ~Observer() {}

    virtual void OnScreenOrientationChanged(
        blink::WebScreenOrientationType orientation) {}

    virtual void OnOrientationChanged(float alpha, float beta, float gamma) {}
    virtual void OnAccelerationChanged(float raw_x, float raw_y, float raw_z,
                                       float x, float y, float z) {}
    virtual void OnRotationRateChanged(float alpha, float beta, float gamma) {}
  };

  virtual void AddObserver(Observer* observer);
  virtual void RemoveObserver(Observer* observer);

  virtual blink::WebScreenOrientationType GetScreenOrientation() const {
    return last_orientation_;
  }

  static bool initialized_;

 protected:
  SensorProvider();

  virtual bool Initialize() = 0;
  virtual void Finish() {}

  virtual void OnScreenOrientationChanged(
      blink::WebScreenOrientationType orientation);

  virtual void OnOrientationChanged(float alpha, float beta, float gamma);
  virtual void OnAccelerationChanged(float raw_x, float raw_y, float raw_z,
                                     float x, float y, float z);
  virtual void OnRotationRateChanged(float alpha, float beta, float gamma);

  std::set<Observer*> observers_;
  blink::WebScreenOrientationType last_orientation_;

 private:
  static scoped_ptr<SensorProvider> instance_;

  DISALLOW_COPY_AND_ASSIGN(SensorProvider);
};

}  // namespace xwalk

#endif  // XWALK_TIZEN_MOBILE_SENSOR_SENSOR_PROVIDER_H_
