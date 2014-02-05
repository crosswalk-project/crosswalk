// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Elementary.h>
#include <Ecore.h>

#include "dialog_launcher.h"

EAPI_MAIN int elm_main(int argc, char **argv) {
  ecore_init();
  e_dbus_init();
  elm_init(argc, argv);
  elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
  DialogLauncher* service = create_service();
  if (register_service(service))
    elm_run();
  shutdown_service(service);
  elm_shutdown();
  e_dbus_shutdown();
  ecore_shutdown();

  return 0;
}

ELM_MAIN()
