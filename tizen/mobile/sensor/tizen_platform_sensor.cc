// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/mobile/sensor/tizen_platform_sensor.h"

#include <math.h>
#include <string>

#include "base/files/file_path.h"
#include "base/logging.h"

namespace xwalk {

TizenPlatformSensor::TizenPlatformSensor()
    : accel_handle_(-1),
      gyro_handle_(-1) {
}

TizenPlatformSensor::~TizenPlatformSensor() {
}

bool TizenPlatformSensor::Initialize() {
  accel_handle_ = sf_connect(ACCELEROMETER_SENSOR);
  if (accel_handle_ >= 0) {
    if (sf_register_event(accel_handle_, ACCELEROMETER_EVENT_ROTATION_CHECK,
                          NULL, OnEventReceived, this) < 0 ||
        sf_register_event(accel_handle_,
                          ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME,
                          NULL, OnEventReceived, this) < 0 ||
        sf_start(accel_handle_, 0) < 0) {
      LOG(ERROR) << "Register accelerometer sensor event failed";
      sf_unregister_event(accel_handle_, ACCELEROMETER_EVENT_ROTATION_CHECK);
      sf_unregister_event(accel_handle_,
                          ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
      sf_disconnect(accel_handle_);
      accel_handle_ = -1;
    }
  } else {
    LOG(ERROR) << "Connection to accelerometer sensor failed";
  }

  gyro_handle_ = sf_connect(GYROSCOPE_SENSOR);
  if (gyro_handle_ >= 0) {
    if (sf_register_event(gyro_handle_, GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME,
                          NULL, OnEventReceived, this) < 0 ||
        sf_start(gyro_handle_, 0) < 0) {
      LOG(ERROR) << "Register gyroscope sensor event failed";
      sf_unregister_event(gyro_handle_,
                          GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME);
      sf_disconnect(gyro_handle_);
      gyro_handle_ = -1;
    }
  } else {
    LOG(ERROR) << "Connection to gyroscope sensor failed";
  }

  return (accel_handle_ >= 0 || gyro_handle_ >= 0);
}

void TizenPlatformSensor::Finish() {
  if (accel_handle_ >= 0) {
    sf_stop(accel_handle_);
    sf_unregister_event(accel_handle_, ACCELEROMETER_EVENT_ROTATION_CHECK);
    sf_unregister_event(accel_handle_,
                        ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
    sf_disconnect(accel_handle_);
    accel_handle_ = -1;
  }

  if (gyro_handle_ >=0) {
    sf_stop(gyro_handle_);
    sf_unregister_event(gyro_handle_, GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME);
    sf_disconnect(gyro_handle_);
    gyro_handle_ = -1;
  }
}

gfx::Display::Rotation TizenPlatformSensor::ToDisplayRotation(
    int rotation) const {
  gfx::Display::Rotation r = gfx::Display::ROTATE_0;
  switch (rotation) {
    case ROTATION_EVENT_0:
      r = gfx::Display::ROTATE_0;
      break;
    case ROTATION_EVENT_90:
      r = gfx::Display::ROTATE_90;
      break;
    case ROTATION_EVENT_180:
      r = gfx::Display::ROTATE_180;
      break;
    case ROTATION_EVENT_270:
      r = gfx::Display::ROTATE_270;
      break;
  }
  return r;
}

void TizenPlatformSensor::OnEventReceived(unsigned int event_type,
                                          sensor_event_data_t* event_data,
                                          void* udata) {
  TizenPlatformSensor* sensor = reinterpret_cast<TizenPlatformSensor*>(udata);
  sensor_data_t* data =
      reinterpret_cast<sensor_data_t*>(event_data->event_data);
  size_t last = event_data->event_data_size / sizeof(sensor_data_t) - 1;

  switch (event_type) {
    case ACCELEROMETER_EVENT_ROTATION_CHECK: {
      gfx::Display::Rotation r = sensor->ToDisplayRotation(
          *reinterpret_cast<int*>(event_data->event_data));
      sensor->OnRotationChanged(r);
      break;
    }
    case ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME: {
      sensor_data_t linear;
      linear.values[0] = linear.values[1] = linear.values[2] = FP_NAN;
      sf_get_data(sensor->accel_handle_,
                  ACCELEROMETER_LINEAR_ACCELERATION_DATA_SET,
                  &linear);
      sensor->OnAccelerationChanged(data[last].values[0],
                                    data[last].values[1],
                                    data[last].values[2],
                                    linear.values[0],
                                    linear.values[1],
                                    linear.values[2]);

      sensor_data_t orient;
      if (sf_get_data(sensor->accel_handle_,
                      ACCELEROMETER_ORIENTATION_DATA_SET,
                      &orient) >= 0) {
        sensor->OnOrientationChanged(orient.values[0],
                                     orient.values[1],
                                     orient.values[2]);
      }
      break;
    }
    case GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME: {
      sensor->OnRotationRateChanged(data[last].values[0],
                                    data[last].values[1],
                                    data[last].values[2]);
    }
  }
}

}  // namespace xwalk
