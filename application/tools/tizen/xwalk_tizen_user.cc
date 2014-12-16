// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/tools/tizen/xwalk_tizen_user.h"

#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <tzplatform_config.h>

#include "base/memory/scoped_ptr.h"

int xwalk_tizen_check_group_users(void) {
  int err = 0;
  struct group grp;
  struct group* current_g;
  int64_t len = sysconf(_SC_GETGR_R_SIZE_MAX);
  if (len == -1)
    len = 1024;

  scoped_ptr<char[]> buffer(new char[len]);
  err = getgrgid_r(getgid(), &grp, buffer.get(), len, &current_g);
  if (err) {
    fprintf(stderr, "group can't be determined");
    fprintf(stderr, "launching an application will not work\n");
    return -EINVAL;
  }

  if ((!current_g) ||
      strcmp(current_g->gr_name, "users")) {
      fprintf(stderr, "group '%s' is not allowed: ",
              current_g ? current_g->gr_name : "<NULL>");
      fprintf(stderr, "launching an application will not work\n");
      return -EINVAL;
  }
  return 0;
}

int xwalk_tizen_check_user_for_xwalk_backend(void) {
  int err = 0;
  struct group grp;
  struct group* current_g;
  int64_t len = sysconf(_SC_GETGR_R_SIZE_MAX);
  if (len == -1)
    len = 1024;

  scoped_ptr<char[]> buffer(new char[len]);
  err = getgrgid_r(getgid(), &grp, buffer.get(), len, &current_g);
  if (err) {
    fprintf(stderr, "group can't be determined");
    fprintf(stderr, "launching an application will not work\n");
    return -EINVAL;
  }

  if ((!current_g) || (
      strcmp(current_g->gr_name, "users") &&
      getuid() != tzplatform_getuid(TZ_SYS_GLOBALAPP_USER))) {
    fprintf(stderr, "group '%s' is not allowed: ",
            current_g ? current_g->gr_name : "<NULL>");
    fprintf(stderr, "launching an application will not work\n");
    return -EINVAL;
  }
  return 0;
}
