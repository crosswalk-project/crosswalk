// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <appcore/appcore-common.h>
#include <pkgmgr-info.h>

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
  fprintf(stderr, "event '%s'\n", event2str(event));

  if (event == AE_TERMINATE) {
    exit(0);
  }
}

int xwalk_appcore_init(int argc, char** argv, const char* name) {
  appcore_ops.cb_app = application_event_cb;
  appcore_ops.data = NULL;

  return appcore_init(name, &appcore_ops, argc, argv);
}

int xwalk_change_cmdline(int argc, char** argv, const char* app_id) {
  // Change /proc/<pid>/cmdline to app exec path. See XWALK-1722 for details.
  gchar* tizen_app_id = g_strconcat("xwalk.", app_id, NULL);
  pkgmgrinfo_appinfo_h handle;
  char* exec_path = NULL;
  if (pkgmgrinfo_appinfo_get_appinfo(tizen_app_id, &handle) != PMINFO_R_OK ||
      pkgmgrinfo_appinfo_get_exec(handle, &exec_path) != PMINFO_R_OK ||
      !exec_path) {
    fprintf(stderr, "Couldn't find exec path for application: %s\n",
            tizen_app_id);
    return -1;
  }

  for (int i = 0; i < argc; ++i)
    memset(argv[i], 0, strlen(argv[i]));
  strncpy(argv[0], exec_path, strlen(exec_path)+1);
  g_free(tizen_app_id);
  pkgmgrinfo_appinfo_destroy_appinfo(handle);
  return 0;
}

char* xwalk_extract_app_id(const char* tizen_app_id) {
  // The Tizen application ID will have a format like
  // "xwalk.crosswalk_32bytes_app_id".
  char* app_id = NULL;
  gchar** tokens = g_strsplit(tizen_app_id, ".", 2);
  if (g_strv_length(tokens) != 2)
    fprintf(stderr, "Invalid Tizen AppID, fallback to Crosswalk AppID.\n");
  else
    app_id = strdup(tokens[1]);

  g_strfreev(tokens);
  return app_id;
}
