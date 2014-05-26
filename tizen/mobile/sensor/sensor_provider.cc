// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/mobile/sensor/sensor_provider.h"

#include "base/logging.h"
#include "xwalk/tizen/mobile/sensor/tizen_platform_sensor.h"

namespace xwalk {

// static
bool SensorProvider::initialized_ = false;

SensorProvider* SensorProvider::GetInstance() {
  if (!initialized_) {
    instance_.reset(new TizenPlatformSensor());
    if (!instance_->Initialize())
      instance_.reset();
  }
  return instance_.get();
}

SensorProvider::SensorProvider()
    : last_rotation_(gfx::Display::ROTATE_0) {
}

SensorProvider::~SensorProvider() {
  Finish();
}

void SensorProvider::AddObserver(Observer* observer) {
  observers_.insert(observer);
}

void SensorProvider::RemoveObserver(Observer* observer) {
  observers_.erase(observer);
}

void SensorProvider::OnRotationChanged(gfx::Display::Rotation rotation) {
  last_rotation_ = rotation;

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

void SensorProvider::OnAccelerationChanged(
    float raw_x, float raw_y, float raw_z,
    float x, float y, float z) {
  std::set<Observer*>::iterator it;
  for (it = observers_.begin(); it != observers_.end(); ++it)
    (*it)->OnAccelerationChanged(raw_x, raw_y, raw_z, x, y, z);
}

void SensorProvider::OnRotationRateChanged(float alpha,
                                           float beta,
                                           float gamma) {
  std::set<Observer*>::iterator it;
  for (it = observers_.begin(); it != observers_.end(); ++it)
    (*it)->OnRotationRateChanged(alpha, beta, gamma);
}

scoped_ptr<SensorProvider> SensorProvider::instance_;

}  // namespace xwalk
