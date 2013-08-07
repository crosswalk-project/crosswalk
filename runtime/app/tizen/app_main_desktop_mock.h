// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_APP_TIZEN_APP_MAIN_DESKTOP_MOCK_H_
#define XWALK_RUNTIME_APP_TIZEN_APP_MAIN_DESKTOP_MOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  APP_DEVICE_ORIENTATION_0 = 0,
  APP_DEVICE_ORIENTATION_90 = 90,
  APP_DEVICE_ORIENTATION_180 = 180,
  APP_DEVICE_ORIENTATION_270 = 270,
} app_device_orientation_e;

typedef struct {
  int service_id;
} service_h;

typedef bool (*app_create_cb) (void *user_data);
typedef void (*app_pause_cb) (void *user_data);
typedef void (*app_resume_cb) (void *user_data);
typedef void (*app_terminate_cb) (void *user_data);
typedef void (*app_service_cb) (service_h service, void *user_data);
typedef void (*app_low_memory_cb) (void *user_data);
typedef void (*app_low_battery_cb) (void *user_data);
typedef void (*app_device_orientation_cb) \
    (app_device_orientation_e orientation, void *user_data);
typedef void (*app_language_changed_cb) (void *user_data);
typedef void (*app_region_format_changed_cb) (void *user_data);

typedef struct {
  app_create_cb create;
  app_terminate_cb terminate;
  app_pause_cb pause;
  app_resume_cb resume;
  app_service_cb service;
  app_low_memory_cb low_memory;
  app_low_battery_cb low_battery;
  app_device_orientation_cb device_orientation;
  app_language_changed_cb language_changed;
  app_region_format_changed_cb region_format_changed;
} app_event_callback_s;

int app_efl_main(int* argc, char*** argv, app_event_callback_s* callback,
                 void* user_data);

void app_efl_exit();

#ifdef __cplusplus
}
#endif

#endif  // XWALK_RUNTIME_APP_TIZEN_APP_MAIN_DESKTOP_MOCK_H_

