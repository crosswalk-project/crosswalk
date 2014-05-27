// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Part of codes are copied from libslp-sensor with original copyright
// and license as below.
//
// Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0

#ifndef XWALK_TIZEN_MOBILE_SENSOR_TIZEN_PLATFORM_SENSOR_H_
#define XWALK_TIZEN_MOBILE_SENSOR_TIZEN_PLATFORM_SENSOR_H_

#include <sensor.h>
#include <vconf.h>

#include "base/native_library.h"
#include "xwalk/tizen/mobile/sensor/sensor_provider.h"

namespace xwalk {

class TizenPlatformSensor : public SensorProvider {
 public:
  TizenPlatformSensor();
  virtual ~TizenPlatformSensor();

  virtual bool Initialize() OVERRIDE;
  virtual void Finish() OVERRIDE;

 private:
  gfx::Display::Rotation ToDisplayRotation(int rotation) const;

  bool auto_rotation_enabled_;
  int accel_handle_;
  int gyro_handle_;

  static void OnEventReceived(unsigned int event_type,
                              sensor_event_data_t* event_data,
                              void* udata);
  static void OnAutoRotationEnabledChanged(keynode_t* node, void* udata);

  DISALLOW_COPY_AND_ASSIGN(TizenPlatformSensor);
};

}  // namespace xwalk

#endif  // XWALK_TIZEN_MOBILE_SENSOR_TIZEN_PLATFORM_SENSOR_H_
