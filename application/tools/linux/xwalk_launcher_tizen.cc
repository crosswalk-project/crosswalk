// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/tools/linux/xwalk_launcher_tizen.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>

#if defined(OS_TIZEN_MOBILE)
#include <appfw/app_manager.h> 
#include <appcore/appcore-common.h>
#include <aul.h>
#endif

static const char* xwalk_service_name = "org.crosswalkproject.Runtime1";
static const char* xwalk_tizen_appcmd_forwarder_iface =
    "org.crosswalkproject.Running.TizenAppCmdForwarder1";

static GDBusProxy* running_app_proxy = NULL;

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

static void handle_app_launch(GVariant* parameters) {
  GVariant* value = g_variant_get_child_value(parameters, 0);
  gchar* app_id;
  g_variant_get(value, "s", &app_id);
  g_print("app_id: %s\n", app_id);

  int ret = aul_open_app(app_id);
  if (ret < 0)
    fprintf(stderr, "Launch app fail:%d\n", ret);
  g_variant_unref(value);
}

static void handle_app_exit() {
  GError* error = NULL;
  GVariant* result = g_dbus_proxy_call_sync(running_app_proxy, "Terminate",
                                            g_variant_new("()"),
                                            G_DBUS_CALL_FLAGS_NONE,
                                            -1, NULL, &error);
  if (!result)
    fprintf(stderr, "Couldn't call 'Launch' method: %s\n", error->message);
}

static void handle_app_kill(GVariant* parameters) {
  GVariant* value = g_variant_get_child_value(parameters, 0);
  gchar* context_id;
  g_variant_get(value, "s", &context_id);
  g_print("context_id: %s\n", context_id);

  int pid;
  std::stringstream(context_id) >> pid;
  g_variant_unref(value);

  if (pid <= 0) {
    fprintf(stderr, "Context ID is wrong: %d\n", pid);
    return;
  } else if (pid == getpid()) {
    fprintf(stderr, "Can't kill with current app context.\n");
    return;
  }

  char* app_id = NULL;
  int ret = app_manager_get_app_id(pid, &app_id);
  if (ret != APP_MANAGER_ERROR_NONE) {
    fprintf(stderr, "Fail to get app_id.\n");
    return;
  }

  app_context_h app_context;
  ret = app_manager_get_app_context(app_id, &app_context);
  if (ret != APP_MANAGER_ERROR_NONE) {
    fprintf(stderr, "Fail to get app_context.\n");
    return;
  }

  ret = app_manager_terminate_app(app_context);
  if (ret != APP_MANAGER_ERROR_NONE)
    fprintf(stderr, "Fail to termniate app.\n");
}

static void on_appcmd_signal(GDBusProxy* proxy,
                             gchar* sender_name,
                             gchar* signal_name,
                             GVariant* parameters,
                             gpointer user_data) {
  gchar *parameters_str;
  parameters_str = g_variant_print(parameters, TRUE);
  g_print (" *** Received Signal: %s: %s\n", signal_name, parameters_str);
  g_free (parameters_str);

  if (!strcmp(signal_name, "OnLaunchCmd")) {
    handle_app_launch(parameters);
  } else if (!strcmp(signal_name, "OnExitCmd")) {
    handle_app_exit();
  } else if (!strcmp(signal_name, "OnKillCmd")) {
    handle_app_kill(parameters);
  } else {
    fprintf(stderr, "Unkown signal received: %s\n", signal_name);
  }
}

int xwalk_init_cmd_receiver(GDBusConnection* connection,
                            const char* app_object_path,
                            GDBusProxy* app_proxy) {
  GError* error = NULL;
  GDBusProxy* cmd_forwarder = g_dbus_proxy_new_sync(
      connection,
      G_DBUS_PROXY_FLAGS_NONE, NULL, xwalk_service_name,
      app_object_path, xwalk_tizen_appcmd_forwarder_iface, NULL, &error);
  if (!cmd_forwarder) {
    g_print("Couldn't create proxy for '%s': %s\n",
            xwalk_tizen_appcmd_forwarder_iface, error->message);
    g_error_free(error);
    return -1;
  }

  g_signal_connect(cmd_forwarder, "g-signal",
                   G_CALLBACK(on_appcmd_signal), NULL);
  running_app_proxy = app_proxy;
  return 0;
}

int xwalk_appcore_init(int argc, char** argv, const char* name) {
  appcore_ops.cb_app = application_event_cb;
  appcore_ops.data = NULL;

  return appcore_init(name, &appcore_ops, argc, argv);
}
