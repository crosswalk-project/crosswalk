// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/app/tizen/app_main_desktop_mock.h"

#include <Elementary.h>

int app_efl_main(int* argc, char*** argv, app_event_callback_s* callback,
                 void* user_data) {
  elm_init(*argc, *argv);
  callback->create(user_data);
  elm_run();
  callback->terminate(user_data);
  return 0;
}

void app_efl_exit() {
  elm_exit();
}
