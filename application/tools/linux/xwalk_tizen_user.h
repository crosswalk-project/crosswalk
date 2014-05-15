// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_LINUX_XWALK_TIZEN_USER_H_
#define XWALK_APPLICATION_TOOLS_LINUX_XWALK_TIZEN_USER_H_

// When developing on Tizen, we log into the device using 'sdb' as the
// 'root' user, when changing to the 'app' user (the user as all Applications
// run) using 'su', the HOME environment variable is still set to '/root', which
// causes applications connecting to the session D-Bus bus to fail, for example.
// This is a Tizen specific workaround.

int xwalk_tizen_check_user_app(void);

#endif  // XWALK_APPLICATION_TOOLS_LINUX_XWALK_TIZEN_USER_H_
