// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/tizen/tizen_platform_sensor.h"

#include <math.h>
#include <string>

#include "base/files/file_path.h"
#include "base/logging.h"

namespace xwalk {

TizenPlatformSensor::TizenPlatformSensor()
    : accel_handle_(-1),
      gyro_handle_(-1),
      dso_(NULL),
      connect_(NULL),
      disconnect_(NULL),
      start_(NULL),
      stop_(NULL),
      register_event_(NULL),
      unregister_event_(NULL),
      get_data_(NULL),
      check_rotation_(NULL) {
}

TizenPlatformSensor::~TizenPlatformSensor() {
}

bool TizenPlatformSensor::Initialize() {
  if (!LoadLibrary())
    return false;

  accel_handle_ = connect_(ACCELEROMETER_SENSOR);
  if (accel_handle_ >= 0) {
    if (register_event_(accel_handle_, ACCELEROMETER_EVENT_ROTATION_CHECK,
                        NULL, OnEventReceived, this) < 0 ||
        register_event_(accel_handle_,
                        ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME,
                        NULL, OnEventReceived, this) < 0 ||
        start_(accel_handle_, 0) < 0) {
      LOG(ERROR) << "Register accelerometer sensor event failed";
      unregister_event_(accel_handle_, ACCELEROMETER_EVENT_ROTATION_CHECK);
      unregister_event_(accel_handle_,
                        ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
      disconnect_(accel_handle_);
      accel_handle_ = -1;
    }
  } else {
    LOG(ERROR) << "Connection to accelerometer sensor failed";
  }

  gyro_handle_ = connect_(GYROSCOPE_SENSOR);
  if (gyro_handle_ >= 0) {
    if (register_event_(gyro_handle_, GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME,
                      NULL, OnEventReceived, this) < 0 ||
        start_(gyro_handle_, 0) < 0) {
      LOG(ERROR) << "Register gyroscope sensor event failed";
      unregister_event_(gyro_handle_, GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME);
      disconnect_(gyro_handle_);
      gyro_handle_ = -1;
    }
  } else {
    LOG(ERROR) << "Connection to gyroscope sensor failed";
  }

  return (accel_handle_ >= 0 || gyro_handle_ >= 0);
}

void TizenPlatformSensor::Finish() {
  if (accel_handle_ >= 0) {
    stop_(accel_handle_);
    unregister_event_(accel_handle_, ACCELEROMETER_EVENT_ROTATION_CHECK);
    unregister_event_(accel_handle_,
                      ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
    disconnect_(accel_handle_);
    accel_handle_ = -1;
  }

  if (gyro_handle_ >=0) {
    unregister_event_(gyro_handle_, GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME);
    disconnect_(gyro_handle_);
    gyro_handle_ = -1;
  }

  UnloadLibrary();
}

bool TizenPlatformSensor::LoadLibrary() {
  if (dso_)
    return true;

  base::FilePath name("libsensor.so.1");
  std::string errmsg;
  dso_ = base::LoadNativeLibrary(name, &errmsg);
  if (!dso_) {
    LOG(ERROR) << "Load " << name.value() << " failed (" << errmsg << ")";
    return false;
  }

  connect_ = (sf_connect)
      base::GetFunctionPointerFromNativeLibrary(dso_, "sf_connect");
  disconnect_ = (sf_disconnect)
      base::GetFunctionPointerFromNativeLibrary(dso_, "sf_disconnect");
  start_ = (sf_start)
      base::GetFunctionPointerFromNativeLibrary(dso_, "sf_start");
  stop_ = (sf_stop)
      base::GetFunctionPointerFromNativeLibrary(dso_, "sf_stop");
  register_event_ = (sf_register_event)
      base::GetFunctionPointerFromNativeLibrary(dso_, "sf_register_event");
  unregister_event_ = (sf_unregister_event)
      base::GetFunctionPointerFromNativeLibrary(dso_, "sf_unregister_event");
  get_data_ = (sf_get_data)
      base::GetFunctionPointerFromNativeLibrary(dso_, "sf_get_data");
  check_rotation_ = (sf_check_rotation)
      base::GetFunctionPointerFromNativeLibrary(dso_, "sf_check_rotation");
  bool rt = connect_ && disconnect_ && start_ && stop_ &&
            register_event_ && unregister_event_ &&
            get_data_ &&check_rotation_;
  if (!rt) {
    LOG(ERROR) << "Incompatible version of " << name.value();
    UnloadLibrary();
  }
  return rt;
}

void TizenPlatformSensor::UnloadLibrary() {
  connect_ = NULL;
  disconnect_ = NULL;
  start_ = NULL;
  stop_ = NULL;
  register_event_ = NULL;
  unregister_event_ = NULL;
  get_data_ = NULL;
  check_rotation_ = NULL;

  if (dso_)
    base::UnloadNativeLibrary(dso_);
  dso_ = NULL;
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
      sensor->get_data_(sensor->accel_handle_,
                        ACCELEROMETER_LINEAR_ACCELERATION_DATA_SET,
                        &linear);
      sensor->OnAccelerationChanged(data[last].values[0],
                                    data[last].values[1],
                                    data[last].values[2],
                                    linear.values[0],
                                    linear.values[1],
                                    linear.values[2]);

      sensor_data_t orient;
      if (sensor->get_data_(sensor->accel_handle_,
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
