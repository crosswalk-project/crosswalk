// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/mobile/sensor/tizen_platform_sensor.h"

#include <math.h>
#include <string>

#include "base/files/file_path.h"
#include "base/logging.h"

namespace  {

// Make the class depend on gfx::Display to avoid the hack below:

#if defined(OS_TIZEN_MOBILE)
int rotation_start = 0;  // Default is portrait primary.
#else
int rotation_start = -1;  // Default is landscape primary.
#endif

blink::WebScreenOrientationType ToScreenOrientation(
    int rotation) {
  rotation = (rotation + rotation_start) % 4;

  blink::WebScreenOrientationType r = blink::WebScreenOrientationUndefined;
  switch (rotation) {
    case ROTATION_EVENT_0:
      r = blink::WebScreenOrientationPortraitPrimary;
      break;
    case ROTATION_EVENT_90:
      r = blink::WebScreenOrientationLandscapeSecondary;
      break;
    case ROTATION_EVENT_180:
      r = blink::WebScreenOrientationPortraitSecondary;
      break;
    case ROTATION_EVENT_270:
      r = blink::WebScreenOrientationLandscapePrimary;
      break;
  }
  return r;
}

}  // namespace

namespace xwalk {

TizenPlatformSensor::TizenPlatformSensor()
    : auto_rotation_enabled_(true),
      accel_handle_(-1),
      gyro_handle_(-1) {
}

TizenPlatformSensor::~TizenPlatformSensor() {
}

bool TizenPlatformSensor::Initialize() {
  unsigned long rotation;  // NOLINT
  if (!sf_check_rotation(&rotation)) {
    last_orientation_ = ToScreenOrientation(static_cast<int>(rotation));
  }

  int value;
  if (!vconf_get_bool(VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL, &value)) {
    auto_rotation_enabled_ = (value != 0);
    vconf_notify_key_changed(VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL,
                             OnAutoRotationEnabledChanged, this);
  }

  accel_handle_ = sf_connect(ACCELEROMETER_SENSOR);
  if (accel_handle_ >= 0) {
    if (sf_register_event(accel_handle_,
            ACCELEROMETER_EVENT_ROTATION_CHECK, NULL,
            OnEventReceived, this) < 0 ||
        sf_register_event(accel_handle_,
            ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME, NULL,
            OnEventReceived, this) < 0 ||
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
            NULL, OnEventReceived, this) < 0 || sf_start(gyro_handle_, 0) < 0) {
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

  vconf_ignore_key_changed(VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL,
      OnAutoRotationEnabledChanged);
}

void TizenPlatformSensor::OnEventReceived(unsigned int event_type,
                                          sensor_event_data_t* event_data,
                                          void* udata) {
  TizenPlatformSensor* self = reinterpret_cast<TizenPlatformSensor*>(udata);

  sensor_data_t* data =
      reinterpret_cast<sensor_data_t*>(event_data->event_data);
  size_t last = event_data->event_data_size / sizeof(sensor_data_t) - 1;

  switch (event_type) {
    case ACCELEROMETER_EVENT_ROTATION_CHECK: {
      if (!self->auto_rotation_enabled_)
        return;
      int value = *reinterpret_cast<int*>(event_data->event_data);
      self->OnScreenOrientationChanged(ToScreenOrientation(value));
      break;
    }
    case ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME: {
      sensor_data_t linear;
      linear.values[0] = linear.values[1] = linear.values[2] = FP_NAN;
      sf_get_data(self->accel_handle_,
          ACCELEROMETER_LINEAR_ACCELERATION_DATA_SET, &linear);
      self->OnAccelerationChanged(
          data[last].values[0], data[last].values[1], data[last].values[2],
          linear.values[0], linear.values[1], linear.values[2]);

      sensor_data_t orient;
      if (sf_get_data(self->accel_handle_,
              ACCELEROMETER_ORIENTATION_DATA_SET, &orient) >= 0) {
        self->OnOrientationChanged(
            orient.values[0], orient.values[1], orient.values[2]);
      }
      break;
    }
    case GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME: {
      self->OnRotationRateChanged(
          data[last].values[0], data[last].values[1], data[last].values[2]);
    }
  }
}

void TizenPlatformSensor::OnAutoRotationEnabledChanged(
    keynode_t* node, void* udata) {
  TizenPlatformSensor* self = reinterpret_cast<TizenPlatformSensor*>(udata);

  self->auto_rotation_enabled_ = (vconf_keynode_get_bool(node) != 0);

  unsigned long value;  // NOLINT
  if (!self->auto_rotation_enabled_) {
    // Change orientation to initial platform orientation when disabled.
    self->OnScreenOrientationChanged(
        ToScreenOrientation(ROTATION_EVENT_0));
  } else if (self->auto_rotation_enabled_ && !sf_check_rotation(&value)) {
    self->OnScreenOrientationChanged(
        ToScreenOrientation(static_cast<int>(value)));
  }
}

}  // namespace xwalk
