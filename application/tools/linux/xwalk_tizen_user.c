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

int xwalk_tizen_set_home_for_user_app(void) {
#if !defined(OS_TIZEN_MOBILE)
  return 0;
#endif

  // Tizen doesn't set HOME by default on login for user "app".
  uid_t uid = getuid();
  struct passwd* passwd = getpwuid(uid);

  if (!passwd)
    return -ENOENT;

  if (strcmp(passwd->pw_name, "app")) {
    fprintf(stderr, "User is not 'app', launching an application will not work\n");
    return -EINVAL;
  }

  if (setenv("HOME", passwd->pw_dir, true) != 0) {
    fprintf(stderr, "Couldn't set 'HOME' env variable to '%s'\n", passwd->pw_dir);
    return -EINVAL;
  }

  return 0;
}
