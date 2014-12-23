// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/tools/linux/dbus_connection.h"

GDBusConnection* get_session_bus_connection(GError** error) {
#if defined(OS_TIZEN_MOBILE)
  // In Tizen the session bus is created in /run/user/app/dbus/user_bus_socket
  // but this information isn't set in DBUS_SESSION_BUS_ADDRESS, neither when
  // logging via 'sdb shell' and changing user to 'app', nor when an application
  // is launched.
  return g_dbus_connection_new_for_address_sync(
      "unix:path=/run/user/app/dbus/user_bus_socket",
      GDBusConnectionFlags(G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT
                           | G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION),
      NULL, NULL, error);
#else
  return g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, error);
#endif
}
