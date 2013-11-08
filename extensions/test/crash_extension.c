// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(__cplusplus)
#error "This file is written in C to make sure the C API works as intended."
#endif

#include <stdio.h>
#include <stdlib.h>
#include "xwalk/extensions/public/XW_Extension.h"
#include "xwalk/extensions/public/XW_Extension_SyncMessage.h"

XW_Extension g_extension = 0;
const XW_CoreInterface* g_core = NULL;
const XW_MessagingInterface* g_messaging = NULL;
const XW_Internal_SyncMessagingInterface* g_sync_messaging = NULL;

void crash() {
  int* die_sweetie_die = NULL;
  *die_sweetie_die = 0xdead;
}

void instance_created(XW_Instance instance) {
}

void instance_destroyed(XW_Instance instance) {
}

void handle_message(XW_Instance instance, const char* message) {
  g_messaging->PostMessage(instance, message);
  crash();
}

void handle_sync_message(XW_Instance instance, const char* message) {
  g_sync_messaging->SetSyncReply(instance, message);
  crash();
}

void shutdown(XW_Extension extension) {
}

int32_t XW_Initialize(XW_Extension extension, XW_GetInterface get_interface) {
  static const char* kAPI =
      "var crashListener = null;"
      "extension.setMessageListener(function(msg) {"
      "  if (crashListener instanceof Function) {"
      "    crashListener(msg);"
      "  };"
      "});"
      "exports.die = function(msg, callback) {"
      "  crashListener = callback;"
      "  extension.postMessage(msg);"
      "};"
      "exports.syncDie = function(msg) {"
      "  return extension.internal.sendSyncMessage(msg);"
      "};";

  g_extension = extension;
  g_core = get_interface(XW_CORE_INTERFACE);
  g_core->SetExtensionName(extension, "crash");
  g_core->SetJavaScriptAPI(extension, kAPI);
  g_core->RegisterInstanceCallbacks(
      extension, instance_created, instance_destroyed);
  g_core->RegisterShutdownCallback(extension, shutdown);

  g_messaging = get_interface(XW_MESSAGING_INTERFACE);
  g_messaging->Register(extension, handle_message);

  g_sync_messaging = get_interface(XW_INTERNAL_SYNC_MESSAGING_INTERFACE);
  g_sync_messaging->Register(extension, handle_sync_message);

  return XW_OK;
}
