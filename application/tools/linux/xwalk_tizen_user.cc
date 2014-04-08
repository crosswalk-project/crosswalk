// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

int xwalk_tizen_check_user_app(void) {
  char* buffer = NULL;
  int err = 0;
  struct group grp;
  struct group* current_g;
  int64_t len = sysconf(_SC_GETGR_R_SIZE_MAX);
  if (len == -1)
    len = 1024;
  buffer = reinterpret_cast<char*>(malloc((size_t)len));
  if (!buffer)
    return -EINVAL;

  err = getgrgid_r(getgid(), &grp, buffer, len, &current_g);
  if (err) {
  fprintf(stderr, "group can't be determined");
    fprintf(stderr, "launching an application will not work\n");
    free(buffer);
    return -EINVAL;
  } else {
    if ( (!current_g) || (
        strcmp(current_g->gr_name, "users") &&
        strcmp(current_g->gr_name, "app"))) {
      fprintf(stderr, "group '%s' is not allowed :",
            current_g ? current_g->gr_name : "<NULL>");
      fprintf(stderr, "launching an application will not work\n");
      free(buffer);
      return -EINVAL;
    }
  }
  return 0;
}
