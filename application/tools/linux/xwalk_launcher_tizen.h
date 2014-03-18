// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_LINUX_XWALK_LAUNCHER_TIZEN_H_
#define XWALK_APPLICATION_TOOLS_LINUX_XWALK_LAUNCHER_TIZEN_H_

#include <gio/gio.h>

int xwalk_appcore_init(int argc, char** argv, const char* name);

void xwalk_start_dbus_service(GDBusConnection* connection);

#endif  // XWALK_APPLICATION_TOOLS_LINUX_XWALK_LAUNCHER_TIZEN_H_
