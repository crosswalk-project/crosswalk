// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_LINUX_DBUS_CONNECTION_H_
#define XWALK_APPLICATION_TOOLS_LINUX_DBUS_CONNECTION_H_

#include <gio/gio.h>

GDBusConnection* get_session_bus_connection(GError** error);

#endif  // XWALK_APPLICATION_TOOLS_LINUX_DBUS_CONNECTION_H_
