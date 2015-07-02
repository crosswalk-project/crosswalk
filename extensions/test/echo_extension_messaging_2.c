// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(__cplusplus)
#error "This file is written in C to make sure the C API works as intended."
#endif

#include <stdio.h>
#include <stdlib.h>
#include "xwalk/extensions/public/XW_Extension.h"
#include "xwalk/extensions/public/XW_Extension_Message_2.h"

XW_Extension g_extension = 0;
const XW_CoreInterface* g_core = NULL;
const XW_MessagingInterface2* g_messaging_2 = NULL;

void instance_created(XW_Instance instance) {
  printf("Instance %d created!\n", instance);
}

void instance_destroyed(XW_Instance instance) {
  printf("Instance %d destroyed!\n", instance);
}

void handle_message(XW_Instance instance, const char* message) {
  g_messaging_2->PostMessage(instance, message);
}

void handle_binary_message(
    XW_Instance instance, const char* message, const size_t size) {
  g_messaging_2->PostBinaryMessage(instance, message, size);
}

void shutdown(XW_Extension extension) {
  printf("Shutdown\n");
}

int32_t XW_Initialize(XW_Extension extension, XW_GetInterface get_interface) {
  static const char* kAPI =
      "var echoListener = null;"
      "var echoBinaryListener = null;"
      "extension.setMessageListener(function(msg) {"
      "  if (echoListener instanceof Function) {"
      "    if (msg instanceof ArrayBuffer) {"
      "      echoBinaryListener(msg);"
      "    } else {"
      "      echoListener(msg);"
      "    }"
      "  }"
      "});"
      "exports.echo = function(msg, callback) {"
      "  echoListener = callback;"
      "  extension.postMessage(msg);"
      "};"
      "exports.echoBinary = function(msg, callback) {"
      "  echoBinaryListener = callback;"
      "  extension.postMessage(msg);"
      "};";

  g_extension = extension;
  g_core = get_interface(XW_CORE_INTERFACE);
  if (g_core == NULL)
    return XW_ERROR;
  g_core->SetExtensionName(extension, "echo2");
  g_core->SetJavaScriptAPI(extension, kAPI);
  g_core->RegisterInstanceCallbacks(
      extension, instance_created, instance_destroyed);
  g_core->RegisterShutdownCallback(extension, shutdown);

  g_messaging_2 = get_interface(XW_MESSAGING_INTERFACE_2);
  if (g_messaging_2 == NULL)
    return XW_ERROR;
  g_messaging_2->Register(extension, handle_message);
  g_messaging_2->RegisterBinaryMesssageCallback(
      extension, handle_binary_message);

  return XW_OK;
}
