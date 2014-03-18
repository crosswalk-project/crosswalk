// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(OS_TIZEN)
#include <appcore/appcore-common.h>
#include <app_manager.h>
#include <aul.h>
#endif

#include "xwalk/application/tools/linux/xwalk_launcher_tizen.h"
#include "xwalk/application/tools/tizen/dom_error.h"

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

const char launcher_dbus_path[] = "/launcher1";

static GDBusNodeInfo *introspection_data = NULL;

// Introspection data for the exported launcher service.
static const gchar introspection_xml[] =
  "<node>"
  "  <interface name='org.crosswalkproject.Launcher.Application1'>"
  "    <method name='GetPid'>"
  "      <arg type='i' name='pid' direction='out'/>"
  "    </method>"
  "    <method name='GetAppIdByPid'>"
  "      <arg type='i' name='pid' direction='in'/>"
  "      <arg type='i' name='error' direction='out'/>"
  "      <arg type='s' name='app_id' direction='out'/>"
  "    </method>"
  "    <method name='LaunchApp'>"
  "      <arg type='s' name='app_id' direction='in'/>"
  "      <arg type='i' name='error' direction='out'/>"
  "    </method>"
  "    <method name='KillApp'>"
  "      <arg type='s' name='context_id' direction='in'/>"
  "      <arg type='i' name='error' direction='out'/>"
  "    </method>"
  "  </interface>"
  "</node>";

static void send_response(GDBusMethodInvocation* invocation,
                          int result) {
  g_dbus_method_invocation_return_value(
      invocation, g_variant_new("(i)", result));
}

static void send_response(GDBusMethodInvocation* invocation,
                          int result, const char* str) {
  g_dbus_method_invocation_return_value(
      invocation, g_variant_new("(is)", result, str));
}

static void handle_get_pid(GDBusMethodInvocation* invocation) {
  send_response(invocation, getpid());
}

static void handle_get_appid_by_pid(GVariant* parameters,
                                    GDBusMethodInvocation* invocation) {
  int pid = -1;
  g_variant_get(parameters, "(i)", &pid);
  if (pid <= 0)
    return send_response(invocation, NOT_FOUND_ERR, "");

  char* app_id = NULL;
  int ret = app_manager_get_app_id(pid, &app_id);
  if (ret == APP_MANAGER_ERROR_NONE && app_id) {
    send_response(invocation, NO_ERROR, app_id);
    free(app_id);
    return;
  }

  fprintf(stderr, "Fail to get app id by pid: %d.\n", ret);
  switch (ret) {
    case APP_MANAGER_ERROR_NO_SUCH_APP:
    case APP_MANAGER_ERROR_INVALID_PARAMETER:
      return send_response(invocation, NOT_FOUND_ERR, "");
    default:
      return send_response(invocation, UNKNOWN_ERR, "");
  }
}

static void handle_app_launch(GVariant* parameters,
                              GDBusMethodInvocation* invocation) {
  const gchar* app_id = NULL;
  g_variant_get(parameters, "(&s)", &app_id);
  if (!app_id || !strlen(app_id))
    return send_response(invocation, INVALID_VALUES_ERR);

  WebApiAPIErrors err_code = NO_ERROR;
  int ret = aul_open_app(app_id);
  if (ret < 0) {
    switch (ret) {
      case AUL_R_EINVAL:
      case AUL_R_ERROR:
        err_code = NOT_FOUND_ERR;
      default:
        err_code = UNKNOWN_ERR;
    }
  }

  send_response(invocation, err_code);
}

static void handle_app_kill(GVariant* parameters,
                            GDBusMethodInvocation* invocation) {
  gint ctx_id = -1;
  g_variant_get(parameters, "(i)", &ctx_id);
  if (ctx_id <= 0 || ctx_id == getpid()) {
    return send_response(invocation, INVALID_VALUES_ERR);
  }

  char* app_id = NULL;
  int ret = app_manager_get_app_id(ctx_id, &app_id);
  if (ret != APP_MANAGER_ERROR_NONE) {
    fprintf(stderr, "Fail to get app id by pid: %d.\n", ret);
    switch (ret) {
      case APP_MANAGER_ERROR_NO_SUCH_APP:
      case APP_MANAGER_ERROR_INVALID_PARAMETER:
        return send_response(invocation, NOT_FOUND_ERR);
      default:
        return send_response(invocation, UNKNOWN_ERR);
    }
  }

  app_context_h app_context;
  ret = app_manager_get_app_context(app_id, &app_context);
  if (ret != APP_MANAGER_ERROR_NONE) {
    fprintf(stderr, "Fail to get app_context: %d.\n", ret);
    return send_response(invocation, NOT_FOUND_ERR);
  }

  ret = app_manager_terminate_app(app_context);
  if (ret != APP_MANAGER_ERROR_NONE) {
    fprintf(stderr, "Fail to termniate app: %d.\n", ret);
    return send_response(invocation, UNKNOWN_ERR);
  }

  return send_response(invocation, NO_ERROR);
}

static void handle_method_call(GDBusConnection* connection,
                               const gchar* sender,
                               const gchar* object_path,
                               const gchar* interface_name,
                               const gchar* method_name,
                               GVariant* parameters,
                               GDBusMethodInvocation* invocation,
                               gpointer user_data) {
  if (!g_strcmp0(method_name, "GetPid")) {
    handle_get_pid(invocation);
  } else if (!g_strcmp0(method_name, "GetAppIdByPid")) {
    handle_get_appid_by_pid(parameters, invocation);
  } else if (!g_strcmp0(method_name, "LaunchApp")) {
    handle_app_launch(parameters, invocation);
  } else if (!g_strcmp0(method_name, "KillApp")) {
    handle_app_kill(parameters, invocation);
  } else {
    fprintf(stderr, "Unkown method called: %s\n", method_name);
  }
}

static const GDBusInterfaceVTable interface_vtable = {
  handle_method_call,
  NULL,
  NULL,
};

void xwalk_start_dbus_service(GDBusConnection* connection) {
  introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, NULL);

  guint registration_id = g_dbus_connection_register_object(
      connection, launcher_dbus_path, introspection_data->interfaces[0],
      &interface_vtable, NULL, NULL, NULL);
  if (!registration_id) {
    fprintf(stderr, "Fail to register launcher object.\n");
    exit(1);
  }
}

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
