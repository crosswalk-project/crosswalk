// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/sensor_provider.h"

#include "base/logging.h"

#if defined(OS_TIZEN_MOBILE)
#include "xwalk/runtime/browser/tizen_platform_sensor.h"
#endif

namespace xwalk {

SensorProvider* SensorProvider::GetInstance() {
  if (!instance_) {
#if defined(OS_TIZEN_MOBILE)
    instance_ = new TizenPlatformSensor();
#endif
    if (instance_ && !instance_->Initialize()) {
      delete instance_;
      instance_ = NULL;
    }
  }
  return instance_;
}

SensorProvider::SensorProvider() {
}

SensorProvider::~SensorProvider() {
  DCHECK(instance_ == this);
  Finish();
  instance_ = NULL;
}

void SensorProvider::AddObserver(Observer* observer) {
  observers_.insert(observer);
}

void SensorProvider::RemoveObserver(Observer* observer) {
    observers_.erase(observer);
}

void SensorProvider::OnRotationChanged(gfx::Display::Rotation rotation) {
  std::set<Observer*>::iterator it;
  for (it = observers_.begin(); it != observers_.end(); ++it)
    (*it)->OnRotationChanged(rotation);
}

void SensorProvider::OnOrientationChanged(float alpha,
                                          float beta,
                                          float gamma) {
  std::set<Observer*>::iterator it;
  for (it = observers_.begin(); it != observers_.end(); ++it)
    (*it)->OnOrientationChanged(alpha, beta, gamma);
}

void SensorProvider::OnAccelerationChanged(float x, float y, float z) {
  std::set<Observer*>::iterator it;
  for (it = observers_.begin(); it != observers_.end(); ++it)
    (*it)->OnOrientationChanged(x, y, z);
}

SensorProvider* SensorProvider::instance_ = NULL;

}  // namespace xwalk
