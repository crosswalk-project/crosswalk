// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_SENSOR_PROVIDER_H_
#define XWALK_RUNTIME_BROWSER_SENSOR_PROVIDER_H_

#include <set>

#include "ui/gfx/display.h"

namespace xwalk {

class SensorProvider {
 public:
  class Observer {
   public:
    virtual void OnRotationChanged(gfx::Display::Rotation r) {}
    virtual void OnOrientationChanged(float alpha, float beta, float gamma) {}
    virtual void OnAccelerationChanged(float x, float y, float z) {}

   protected:
    virtual ~Observer() {}
  };

  static SensorProvider* GetInstance();

  virtual void AddObserver(Observer* observer) {
    observers_.insert(observer);
  }
  virtual void RemoveObserver(Observer* observer) {
    observers_.erase(observer);
  }

 protected:
  SensorProvider();
  virtual ~SensorProvider();

  virtual bool Initialize() = 0;
  virtual void Finish() {}

  virtual void OnRotationChanged(gfx::Display::Rotation rotation) {
    std::set<Observer*>::iterator it;
    for (it = observers_.begin(); it != observers_.end(); ++it)
      (*it)->OnRotationChanged(rotation);
  }
  virtual void OnOrientationChanged(float alpha, float beta, float gamma) {
    std::set<Observer*>::iterator it;
    for (it = observers_.begin(); it != observers_.end(); ++it)
      (*it)->OnOrientationChanged(alpha, beta, gamma);
  }
  virtual void OnAccelerationChanged(float x, float y, float z) {
    std::set<Observer*>::iterator it;
    for (it = observers_.begin(); it != observers_.end(); ++it)
      (*it)->OnOrientationChanged(x, y, z);
  }

  std::set<Observer*> observers_;

 private:
  static SensorProvider* instance_;

  DISALLOW_COPY_AND_ASSIGN(SensorProvider);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_SENSOR_PROVIDER_H_
