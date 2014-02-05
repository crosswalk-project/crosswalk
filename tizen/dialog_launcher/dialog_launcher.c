// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>

#include <Elementary.h>
#include <Ecore.h>
#include <E_DBus.h>

#include "dialog_launcher.h"

struct DialogLauncher {
  E_DBus_Connection* conn;
};

const char kDialogLauncherServiceName[] = "org.crosswalkproject.DialogLauncher";
const char kDialogLauncherInterfaceName[] = "org.crosswalkproject.DialogLauncher";
const char kDialogLauncherObjectPath[] = "/org/crosswalkproject/DialogLauncher";

static void request_name_cb(void *data, DBusMessage *msg, DBusError *err);

DialogLauncher* create_service() {
  return (DialogLauncher*)calloc(1, sizeof(DialogLauncher));
}

Eina_Bool register_service(DialogLauncher* service) {
  EINA_SAFETY_ON_NULL_RETURN_VAL(service, EINA_FALSE);
  service->conn = e_dbus_bus_get(DBUS_BUS_SESSION);
  if (!service->conn)
    return EINA_FALSE;

  e_dbus_request_name(service->conn, kDialogLauncherServiceName, 0,
                      request_name_cb, NULL);
  E_DBus_Object* dialog_launcher = e_dbus_object_add(service->conn,
                                                     kDialogLauncherObjectPath,
                                                     NULL);
  E_DBus_Interface* interface =
                             e_dbus_interface_new(kDialogLauncherInterfaceName);
  register_date_time_launcher(service->conn, interface);
  // TODO(shalamov): register other dialog launchers
  e_dbus_object_interface_attach(dialog_launcher, interface);
  return EINA_TRUE;
}

static void request_name_cb(void *data, DBusMessage *msg, DBusError *err) {
  if (dbus_error_is_set(err)) {
    EINA_LOG_ERR("%s", "Cannot register service name.");
    elm_exit();
  }
}

void shutdown_service(DialogLauncher* service) {
  EINA_SAFETY_ON_NULL_RETURN(service);
  if (service) {
    e_dbus_connection_close(service->conn);
    free(service);
  }
}
