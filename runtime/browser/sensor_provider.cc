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

SensorProvider* SensorProvider::instance_ = NULL;

}  // namespace xwalk
