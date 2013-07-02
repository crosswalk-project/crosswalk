// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(__cplusplus)
#error "This file is written in C to make sure the C API works as intended."
#endif

#include <stdio.h>
#include <stdlib.h>
#include "cameo/extensions/public/cameo_extension_public.h"

#if defined(_WIN32)
#include <windows.h>
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

static void context_handle_message(CCameoExtensionContext* context,
                                    const char *message) {
  cameo_extension_context_post_message(context, message);
}

static void context_destroy(CCameoExtensionContext* context) {
  free(context);
}

static CCameoExtensionContext* context_create(CCameoExtension* extension) {
  CCameoExtensionContext* context = calloc(1, sizeof(*context));
  if (!context)
    return NULL;

  context->destroy = context_destroy;
  context->handle_message = context_handle_message;

  return context;
}

static const char* get_javascript(CCameoExtension* extension) {
  static const char* kAPI =
      "var cameo = cameo || {};"
      "cameo.setMessageListener('echo', function(msg) {"
      "  if (cameo.echoListener instanceof Function) {"
      "    cameo.echoListener(msg);"
      "  };"
      "});"
      "cameo.echo = function(msg, callback) {"
      "  cameo.echoListener = callback;"
      "  cameo.postMessage('echo', msg);"
      "};";
  return kAPI;
}

static CCameoExtension extension = {
  .name = "echo",
  .api_version = 1,
  .get_javascript = get_javascript,
  .shutdown = NULL,
  .context_create = context_create
};

EXPORT CCameoExtension* cameo_extension_init(int32_t api_version) {
  return &extension;
}
