// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(__cplusplus)
#error "This file is written in C to make sure the C API works as intended."
#endif

#include <stdio.h>
#include <stdlib.h>
#include "xwalk/extensions/public/XW_Extension.h"
#include "xwalk/extensions/public/XW_Extension_Runtime.h"
#include "xwalk/extensions/public/XW_Extension_SyncMessage.h"

static XW_Extension g_extension;
static const XW_CoreInterface* g_core;
static const XW_MessagingInterface* g_messaging;
static const XW_Internal_SyncMessagingInterface* g_sync_messaging;
static const XW_Internal_RuntimeInterface* g_runtime;

void instance_created(XW_Instance instance) {
  printf("Instance %d created!\n", instance);
}

void instance_destroyed(XW_Instance instance) {
  printf("Instance %d destroyed!\n", instance);
}

void handle_message(XW_Instance instance, const char* message) {
  g_messaging->PostMessage(instance, message);
}

void handle_sync_message(XW_Instance instance, const char* message) {
  static char out[8192];
  g_runtime->GetRuntimeVariableString(g_extension, message, out, sizeof(out));
  g_sync_messaging->SetSyncReply(instance, out);
}

void shutdown(XW_Extension extension) {
  printf("Shutdown\n");
}

int32_t XW_Initialize(XW_Extension extension, XW_GetInterface get_interface) {
  static const char* kAPI =
      "exports.getVariable = function(name) {"
      "  return JSON.parse(extension.internal.sendSyncMessage(name));"
      "};";

  g_extension = extension;
  g_core = get_interface(XW_CORE_INTERFACE);
  g_core->SetExtensionName(extension, "runtime");
  g_core->SetJavaScriptAPI(extension, kAPI);
  g_core->RegisterInstanceCallbacks(
      extension, instance_created, instance_destroyed);
  g_core->RegisterShutdownCallback(extension, shutdown);

  g_messaging = get_interface(XW_MESSAGING_INTERFACE);
  g_messaging->Register(extension, handle_message);

  g_sync_messaging = get_interface(XW_INTERNAL_SYNC_MESSAGING_INTERFACE);
  g_sync_messaging->Register(extension, handle_sync_message);

  g_runtime = get_interface(XW_INTERNAL_RUNTIME_INTERFACE);
  if (!g_runtime)
    return XW_ERROR;

  return XW_OK;
}
