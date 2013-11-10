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

#ifndef XWALK_RUNTIME_BROWSER_TIZEN_TIZEN_PLATFORM_SENSOR_H_
#define XWALK_RUNTIME_BROWSER_TIZEN_TIZEN_PLATFORM_SENSOR_H_

#include "base/native_library.h"
#include "xwalk/runtime/browser/tizen/sensor_provider.h"

namespace xwalk {

class TizenPlatformSensor : public SensorProvider {
 public:
  TizenPlatformSensor();
  virtual ~TizenPlatformSensor();

  virtual bool Initialize() OVERRIDE;
  virtual void Finish() OVERRIDE;

 private:
  bool LoadLibrary();
  void UnloadLibrary();
  gfx::Display::Rotation ToDisplayRotation(int rotation) const;

  int accel_handle_;
  int gyro_handle_;
  base::NativeLibrary dso_;

  // Start of codes copied from libslp-sensor
  typedef enum {
    UNKNOWN_SENSOR       = 0x0000,
    ACCELEROMETER_SENSOR = 0x0001,
    GEOMAGNETIC_SENSOR   = 0x0002,
    LIGHT_SENSOR         = 0x0004,
    PROXIMITY_SENSOR     = 0x0008,
    THERMOMETER_SENSOR   = 0x0010,
    GYROSCOPE_SENSOR     = 0x0020,
    PRESSURE_SENSOR      = 0x0040,
    MOTION_SENSOR        = 0x0080,
  } sensor_type_t;

  enum accelerometer_data_id {
    ACCELEROMETER_BASE_DATA_SET =
        (ACCELEROMETER_SENSOR << 16) | 0x0001,
    ACCELEROMETER_ORIENTATION_DATA_SET =
        (ACCELEROMETER_SENSOR << 16) | 0x0002,
    ACCELEROMETER_LINEAR_ACCELERATION_DATA_SET =
        (ACCELEROMETER_SENSOR << 16) | 0x0004,
    ACCELEROMETER_GRAVITY_DATA_SET =
        (ACCELEROMETER_SENSOR << 16) | 0x0008,
  };

  enum accelerometer_event_type {
    ACCELEROMETER_EVENT_ROTATION_CHECK =
        (ACCELEROMETER_SENSOR << 16) | 0x0001,
    ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME =
        (ACCELEROMETER_SENSOR << 16) | 0x0002,
    ACCELEROMETER_EVENT_CALIBRATION_NEEDED =
        (ACCELEROMETER_SENSOR << 16) | 0x0004,
    ACCELEROMETER_EVENT_SET_HORIZON =
        (ACCELEROMETER_SENSOR << 16) | 0x0008,
    ACCELEROMETER_EVENT_SET_WAKEUP =
        (ACCELEROMETER_SENSOR << 16) | 0x0010,
    ACCELEROMETER_EVENT_ORIENTATION_DATA_REPORT_ON_TIME =
        (ACCELEROMETER_SENSOR << 16) | 0x0020,
    ACCELEROMETER_EVENT_LINEAR_ACCELERATION_DATA_REPORT_ON_TIME =
        (ACCELEROMETER_SENSOR << 16) | 0x0040,
    ACCELEROMETER_EVENT_GRAVITY_DATA_REPORT_ON_TIME =
        (ACCELEROMETER_SENSOR << 16) | 0x0080,
  };

  enum accelerometer_rotate_state {
    ROTATION_UNKNOWN         = 0,
    ROTATION_LANDSCAPE_LEFT  = 1,
    ROTATION_PORTRAIT_TOP    = 2,
    ROTATION_PORTRAIT_BTM    = 3,
    ROTATION_LANDSCAPE_RIGHT = 4,
    ROTATION_EVENT_0         = 2,
    ROTATION_EVENT_90        = 1,
    ROTATION_EVENT_180       = 3,
    ROTATION_EVENT_270       = 4,
  };

  enum gyro_data_id {
    GYRO_BASE_DATA_SET = (GYROSCOPE_SENSOR << 16) | 0x0001,
  };

  enum gyro_event_type {
    GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME = (GYROSCOPE_SENSOR << 16) | 0x0001,
  };

  typedef enum {
    CONDITION_NO_OP,
    CONDITION_EQUAL,
    CONDITION_GREAT_THAN,
    CONDITION_LESS_THAN,
  } condition_op_t;

  typedef struct {
    condition_op_t cond_op;
    float cond_value1;
  } event_condition_t;

  typedef struct {
    size_t event_data_size;
    void* event_data;
  } sensor_event_data_t;

  typedef struct {
    int data_accuracy;
    int data_unit_idx;
    unsigned long long time_stamp;  // NOLINT
    int values_num;
    float values[12];
  } sensor_data_t;

  typedef void (*sensor_callback_func_t)(unsigned int event_type,
                                         sensor_event_data_t* event_data,
                                         void* udata);

  typedef int (*sf_connect)(int sensor_type);
  typedef int (*sf_disconnect)(int handle);
  typedef int (*sf_start)(int handle, int option);
  typedef int (*sf_stop)(int handle);
  typedef int (*sf_register_event)(int handle,
                                   unsigned int event_type,
                                   event_condition_t* event_condition,
                                   sensor_callback_func_t cb,
                                   void* cb_data);
  typedef int (*sf_unregister_event)(int handle, int event_type);
  typedef int (*sf_get_data)(int handle,
                             unsigned int data_id,
                             sensor_data_t* values);
  typedef int (*sf_check_rotation)(unsigned long* curr_state);  // NOLINT
  // End of codes copied from libslp-sensor

  sf_connect connect_;
  sf_disconnect disconnect_;
  sf_start start_;
  sf_stop stop_;
  sf_register_event register_event_;
  sf_unregister_event unregister_event_;
  sf_get_data get_data_;
  sf_check_rotation check_rotation_;

  static void OnEventReceived(unsigned int event_type,
                              sensor_event_data_t* event_data,
                              void* udata);

  DISALLOW_COPY_AND_ASSIGN(TizenPlatformSensor);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_TIZEN_TIZEN_PLATFORM_SENSOR_H_
