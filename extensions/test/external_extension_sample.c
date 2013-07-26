// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(__cplusplus)
#error "This file is written in C to make sure the C API works as intended."
#endif

#include <stdio.h>
#include <stdlib.h>
#include "xwalk/extensions/public/xwalk_extension_public.h"

static void context_handle_message(CXWalkExtensionContext* context,
                                    const char *message) {
  xwalk_extension_context_post_message(context, message);
}

static void context_handle_sync_message(CXWalkExtensionContext* context,
					const char *message) {
  xwalk_extension_context_set_sync_reply(context, message);
}

static void context_destroy(CXWalkExtensionContext* context) {
  free(context);
}

static CXWalkExtensionContext* context_create(CXWalkExtension* extension) {
  CXWalkExtensionContext* context = calloc(1, sizeof(*context));
  if (!context)
    return NULL;

  context->destroy = context_destroy;
  context->handle_message = context_handle_message;
  context->handle_sync_message = context_handle_sync_message;

  return context;
}

static const char* get_javascript(CXWalkExtension* extension) {
  static const char* kAPI =
      "var echoListener = null;"
      "extension.setMessageListener(function(msg) {"
      "  if (echoListener instanceof Function) {"
      "    echoListener(msg);"
      "  };"
      "});"
      "exports.echo = function(msg, callback) {"
      "  echoListener = callback;"
      "  extension.postMessage(msg);"
      "};"
      "exports.syncEcho = function(msg) {"
      "  return extension.internal.sendSyncMessage(msg);"
      "};";
  return kAPI;
}

static void shutdown(CXWalkExtension* extension) {
  free(extension);
}

CXWalkExtension* xwalk_extension_init(int32_t api_version) {
  CXWalkExtension* extension = calloc(1, sizeof(*extension));
  extension->name = "echo";
  extension->api_version = 1;
  extension->get_javascript = get_javascript;
  extension->shutdown = shutdown;
  extension->context_create = context_create;
  return extension;
}
