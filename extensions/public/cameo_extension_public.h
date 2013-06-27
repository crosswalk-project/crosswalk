// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_EXTENSIONS_PUBLIC_CAMEO_EXTENSION_PUBLIC_H_
#define CAMEO_EXTENSIONS_PUBLIC_CAMEO_EXTENSION_PUBLIC_H_

#ifndef INTERNAL_IMPLEMENTATION
#include <assert.h>
#endif  // INTERNAL_IMPLEMENTATION

#include <stdint.h>

typedef struct CCameoExtension_           CCameoExtension;
typedef struct CCameoExtensionContext_    CCameoExtensionContext;
typedef struct CCameoExtensionContextAPI_ CCameoExtensionContextAPI;

typedef void (*ExtensionShutdownCallback)(CCameoExtension* extension);
typedef const char* (*ExtensionGetJavaScriptCallback)(
      CCameoExtension* extension);
typedef CCameoExtensionContext* (*ExtensionContextCreateCallback)(
      CCameoExtension* extension);

typedef void (*ExtensionContextDestroyCallback)(
      CCameoExtensionContext* context);
typedef void (*ExtensionContextHandleMessageCallback)(
      CCameoExtensionContext* context, const char* message);

typedef void (*ExtensionContextPostMessageCallback)(
      CCameoExtensionContext* context, const char* message);

struct CCameoExtension_ {
  int32_t api_version;

  // Version 1
  const char* name;

  ExtensionGetJavaScriptCallback get_javascript;
  ExtensionShutdownCallback shutdown;
  ExtensionContextCreateCallback context_create;
};

struct CCameoExtensionContextAPI_ {
  // Version 1
  ExtensionContextPostMessageCallback post_message;
};

struct CCameoExtensionContext_ {
  void* internal_data;
  const CCameoExtensionContextAPI* api;

  // Version 1
  ExtensionContextDestroyCallback destroy;
  ExtensionContextHandleMessageCallback handle_message;
};

#ifndef INTERNAL_IMPLEMENTATION
// This function should be implemented and exported in the shared
// object. The ``api_version'' parameter will contain the maximum
// version supported by Cameo.
// On a successful invocation, this function should return a pointer
// to a CCameoExtension structure, with the fields:
// - api_version, filled with the API version the extension implements.
// - name, with the extension name (used by cameo.postMessage() and
//   friends).
// - get_javascript, filled with a pointer to a function that returns
//   the JavaScript shim to be available in all page contexts.
// - shutdown, filled with a pointer to a function that is called
//   whenever this extension is shut down (e.g. Cameo terminating). NULL
//   is fine.
// - context_create, filled with a pointer to a function that creates
//   an extension context (see comment below).
CCameoExtension* cameo_extension_init(int32_t api_version);

// A CCameoExtension structure holds the global state for a extension.
// Due to the multithreaded way Cameo is written, one should not
// store mutable state there. That's the reason CCameoExtensionContext
// exists: so each page context has its own state and there's no need
// to worry about race conditions while keeping state between contexts.
//
// The first two fields of a context (internal_data and api) should not
// be tampered with. The following fields, though, should be filled:
// - destroy, with a pointer to a function that will be called whenever
//   a particular context is about to be destroyed.
// - handle_message, with a pointer to a function that will be called
//   whenever a message arrives from the JavaScript side.
//
// To post a message to the JavaScript side, one can simply call
// cameo_extension_context_post_message(), defined below. Cameo will
// handle all the multithreading details so it is safe to call this
// whenever necessary.
static void cameo_extension_context_post_message(
      CCameoExtensionContext* context, const char* message) {
  assert(context);
  assert(context->api);
  assert(context->api->post_message);
  assert(message);

  context->api->post_message(context, message);
}
#endif  // INTERNAL_IMPLEMENTATION

#endif  // CAMEO_EXTENSIONS_PUBLIC_CAMEO_EXTENSION_PUBLIC_H_
