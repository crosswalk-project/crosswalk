// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This extension will post messages for incorrect instances. External
// extensions should be treated as "user input" and not trusted. When possible,
// Crosswalk should not crash in case of bad behavior from external extensions.

#if defined(__cplusplus)
#error "This file is written in C to make sure the C API works as intended."
#endif

#include <stdlib.h>
#include <stdio.h>
#include "xwalk/extensions/public/XW_Extension.h"

XW_Extension g_extension = 0;
const XW_CoreInterface* g_core = NULL;
const XW_MessagingInterface* g_messaging = NULL;

XW_Instance g_last_instance_destroyed = 0;

char* kDoneMessage = "DONE";

void OnInstanceCreated(XW_Instance instance) {
  void* data = kDoneMessage;

  // BAD: Sending messages to invalid instances.
  g_messaging->PostMessage(0, "Zero is never a valid instance");
  g_messaging->PostMessage(instance + 1, "Wrong instance");
  g_messaging->PostMessage(instance + 1000, "Another wrong instance");
  g_messaging->PostMessage(g_last_instance_destroyed, "Destroyed instance");

  // BAD: Setting extra data for invalid instances.
  g_core->SetInstanceData(0, NULL);
  g_core->SetInstanceData(instance + 1, NULL);
  g_core->SetInstanceData(instance + 1000, NULL);
  g_core->SetInstanceData(g_last_instance_destroyed, NULL);

  g_core->SetInstanceData(instance, data);
}

void OnInstanceDestroyed(XW_Instance instance) {
  g_last_instance_destroyed = instance;
}

void OnMessageReceived(XW_Instance instance, const char* message) {
  char* done_message = NULL;
  void* data = NULL;

  // BAD: Sending messages to invalid instances.
  g_messaging->PostMessage(0, "Zero is never a valid instance");
  g_messaging->PostMessage(instance + 1, "Wrong instance");
  g_messaging->PostMessage(instance + 1000, "Another wrong instance");
  g_messaging->PostMessage(g_last_instance_destroyed, "Destroyed instance");

  // BAD: Getting extra data for invalid instances.
  data = g_core->GetInstanceData(0);
  data = g_core->GetInstanceData(instance + 1);
  data = g_core->GetInstanceData(instance + 1000);
  data = g_core->GetInstanceData(g_last_instance_destroyed);

  // Send a correct message to finish the test.
  done_message = g_core->GetInstanceData(instance);
  g_messaging->PostMessage(instance, done_message);
}

int32_t XW_Initialize(XW_Extension extension, XW_GetInterface get_interface) {
  static const char* kAPI =
      "exports.test = function() {"
      "  extension.postMessage('TEST');"
      "};"
      "extension.setMessageListener(function(msg) {"
      "  if (msg == 'DONE') {"
      "    document.title = 'Pass';"
      "  } else {"
      "    document.title = 'Fail';"
      "  }"
      "});";
  g_extension = extension;
  g_core = get_interface(XW_CORE_INTERFACE);

  g_core->SetExtensionName(extension, "bad");
  g_core->SetJavaScriptAPI(extension, kAPI);
  g_core->RegisterInstanceCallbacks(
      extension, OnInstanceCreated, NULL);

  // BAD: Use wrong extension value when setting extension name.
  g_core->SetExtensionName(0, "bad");
  g_core->SetExtensionName(extension + 1, "bad");
  g_core->SetExtensionName(extension + 1000, "bad");

  /* // BAD: Use wrong extension value to register instance callbacks. */
  g_core->RegisterInstanceCallbacks(0, OnInstanceCreated, NULL);
  g_core->RegisterInstanceCallbacks(extension + 1, OnInstanceCreated, NULL);
  g_core->RegisterInstanceCallbacks(extension + 1000, OnInstanceCreated, NULL);

  g_messaging = get_interface(XW_MESSAGING_INTERFACE);
  g_messaging->Register(extension, OnMessageReceived);

  return XW_OK;
}
