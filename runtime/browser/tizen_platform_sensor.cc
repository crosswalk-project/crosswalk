// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/tizen_platform_sensor.h"

#include <string>

#include "base/files/file_path.h"
#include "base/logging.h"

namespace xwalk {

TizenPlatformSensor::TizenPlatformSensor()
    : handle_(-1),
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

  handle_ = connect_(ACCELEROMETER_SENSOR);
  if (handle_ < 0) {
    LOG(ERROR) << "Connection to accelerometer sensor failed";
    return false;
  }

  if (register_event_(handle_, ACCELEROMETER_EVENT_ROTATION_CHECK,
                      NULL, OnEventReceived, this) >= 0 &&
      register_event_(handle_, ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME,
                      NULL, OnEventReceived, this) >= 0) {
    if (start_(handle_, 0) >= 0)
      return true;
    unregister_event_(handle_, ACCELEROMETER_EVENT_ROTATION_CHECK);
    unregister_event_(handle_, ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
  }
  LOG(ERROR) << "Register sensor handler failed";
  disconnect_(handle_);
  handle_ = -1;
  return false;
}

void TizenPlatformSensor::Finish() {
  if (handle_ < 0)
    return;

  stop_(handle_);
  unregister_event_(handle_, ACCELEROMETER_EVENT_ROTATION_CHECK);
  unregister_event_(handle_, ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
  disconnect_(handle_);
  handle_ = -1;

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

  switch (event_type) {
    case ACCELEROMETER_EVENT_ROTATION_CHECK: {
      int* data = reinterpret_cast<int*>(event_data->event_data);
      gfx::Display::Rotation r = sensor->ToDisplayRotation(*data);
      sensor->OnRotationChanged(r);
      break;
    }
    case ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME: {
      sensor_data_t* accel =
          reinterpret_cast<sensor_data_t*>(event_data->event_data);
      size_t cnt = event_data->event_data_size / sizeof(sensor_data_t);
      for (size_t i = 0; i < cnt; i++) {
        sensor->OnAccelerationChanged(accel[i].values[0],
                                      accel[i].values[1],
                                      accel[i].values[2]);
      }

      sensor_data_t data;
      if (sensor->get_data_(sensor->handle_,
                            ACCELEROMETER_ORIENTATION_DATA_SET,
                            &data) >= 0) {
        sensor->OnOrientationChanged(data.values[0],
                                     data.values[1],
                                     data.values[2]);
      }
      break;
    }
  }
}

}  // namespace xwalk
