// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>

#include <Ecore.h>
#include <E_DBus.h>

Eina_Bool launch_dialog(void* conn)
{
  DBusMessage *msg = dbus_message_new_method_call(
    "org.crosswalkproject.DialogLauncher",
    "/org/crosswalkproject/DialogLauncher",
    "org.crosswalkproject.DialogLauncher",
    "OpenDateTimeDialog"
  );

  dbus_int32_t uid = 1;
  dbus_int32_t type = 8;
  dbus_int32_t year = 2013;
  dbus_int32_t month = 11;
  dbus_int32_t day = 11;
  dbus_int32_t hour = 12;
  dbus_int32_t minute = 30;
  dbus_int32_t second = 45;
  dbus_int32_t millisecond = 10;
  dbus_int32_t week = 44;
  dbus_int32_t minimum = 1900;
  dbus_int32_t maximum = 2020;
  dbus_int32_t step = 1;

  dbus_message_append_args(msg, DBUS_TYPE_INT32, &uid,
                                DBUS_TYPE_INT32, &type,
                                DBUS_TYPE_INT32, &year,
                                DBUS_TYPE_INT32, &month,
                                DBUS_TYPE_INT32, &day,
                                DBUS_TYPE_INT32, &hour,
                                DBUS_TYPE_INT32, &minute,
                                DBUS_TYPE_INT32, &second,
                                DBUS_TYPE_INT32, &millisecond,
                                DBUS_TYPE_INT32, &week,
                                DBUS_TYPE_INT32, &maximum,
                                DBUS_TYPE_INT32, &minimum,
                                DBUS_TYPE_INT32, &step,
                                DBUS_TYPE_INVALID);
  e_dbus_message_send((E_DBus_Connection*)conn, msg, NULL, -1, NULL);
  dbus_message_unref(msg);
  return ECORE_CALLBACK_CANCEL;
}

int main()
{
  E_DBus_Connection *conn;
  int ret = 0;
  ecore_init();
  e_dbus_init();
  if (conn = e_dbus_bus_get(DBUS_BUS_SESSION))
    ecore_timer_add(0.0, launch_dialog, conn);
  ecore_main_loop_begin();
  e_dbus_shutdown();
  ecore_shutdown();
  return 0;
}
