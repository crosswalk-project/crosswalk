// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#if defined(OS_TIZEN_MOBILE)
#include <appcore/appcore-common.h>
#endif
#include "xwalk/application/tools/linux/xwalk_launcher_tizen.h"

enum app_event {
  AE_UNKNOWN,
  AE_CREATE,
  AE_TERMINATE,
  AE_PAUSE,
  AE_RESUME,
  AE_RESET,
  AE_LOWMEM_POST,
  AE_MEM_FLUSH,
  AE_MAX
};

// Private struct from appcore-internal, necessary to get events from
// the system.
struct ui_ops {
  void* data;
  void (*cb_app)(enum app_event evnt, void* data, bundle* b);
};

static struct ui_ops appcore_ops;

static const char* event2str(enum app_event event) {
  switch (event) {
    case AE_UNKNOWN:
      return "AE_UNKNOWN";
    case AE_CREATE:
      return "AE_CREATE";
    case AE_TERMINATE:
      return "AE_TERMINATE";
    case AE_PAUSE:
      return "AE_PAUSE";
    case AE_RESUME:
      return "AE_RESUME";
    case AE_RESET:
      return "AE_RESET";
    case AE_LOWMEM_POST:
      return "AE_LOWMEM_POST";
    case AE_MEM_FLUSH:
      return "AE_MEM_FLUSH";
    case AE_MAX:
      return "AE_MAX";
  }

  return "INVALID EVENT";
}

static void application_event_cb(enum app_event event, void* data, bundle* b) {
  fprintf(stderr, "event %s\n", event2str(event));
}

int xwalk_appcore_init(int argc, char** argv, const char* name) {
  appcore_ops.cb_app = application_event_cb;
  appcore_ops.data = NULL;

  return appcore_init(name, &appcore_ops, argc, argv);
}
