// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_PUBLIC_XWALK_EXTENSION_PUBLIC_H_
#define XWALK_EXTENSIONS_PUBLIC_XWALK_EXTENSION_PUBLIC_H_

#ifndef INTERNAL_IMPLEMENTATION
#include <assert.h>
#endif  // INTERNAL_IMPLEMENTATION

#include <stdint.h>

typedef struct CXWalkExtension_           CXWalkExtension;
typedef struct CXWalkExtensionContext_    CXWalkExtensionContext;
typedef struct CXWalkExtensionContextAPI_ CXWalkExtensionContextAPI;

typedef void (*ExtensionShutdownCallback)(CXWalkExtension* extension);
typedef const char* (*ExtensionGetJavaScriptCallback)(
      CXWalkExtension* extension);
typedef CXWalkExtensionContext* (*ExtensionContextCreateCallback)(
      CXWalkExtension* extension);

typedef void (*ExtensionContextDestroyCallback)(
      CXWalkExtensionContext* context);
typedef void (*ExtensionContextHandleMessageCallback)(
      CXWalkExtensionContext* context, const char* message);
typedef void (*ExtensionContextHandleSyncMessageCallback)(
      CXWalkExtensionContext* context, const char* message);

typedef void (*ExtensionContextPostMessageCallback)(
      CXWalkExtensionContext* context, const char* message);
typedef void (*ExtensionContextSetSyncReplyCallback)(
      CXWalkExtensionContext* context, const char* message);

struct CXWalkExtension_ {
  int32_t api_version;

  // Version 1
  const char* name;

  ExtensionGetJavaScriptCallback get_javascript;
  ExtensionShutdownCallback shutdown;
  ExtensionContextCreateCallback context_create;
};

struct CXWalkExtensionContextAPI_ {
  // Version 1
  ExtensionContextPostMessageCallback post_message;
  ExtensionContextSetSyncReplyCallback set_sync_reply;
};

struct CXWalkExtensionContext_ {
  void* internal_data;
  const CXWalkExtensionContextAPI* api;

  // Version 1
  ExtensionContextDestroyCallback destroy;
  ExtensionContextHandleMessageCallback handle_message;
  ExtensionContextHandleSyncMessageCallback handle_sync_message;
};

#ifndef INTERNAL_IMPLEMENTATION
// This function should be implemented and exported in the shared
// object. The ``api_version'' parameter will contain the maximum
// version supported by Crosswalk.
// On a successful invocation, this function should return a pointer
// to a CXWalkExtension structure, with the fields:
// - api_version, filled with the API version the extension implements.
// - name, with the extension name (used by xwalk.postMessage() and
//   friends).
// - get_javascript, filled with a pointer to a function that returns
//   the JavaScript shim to be available in all page contexts.
// - shutdown, filled with a pointer to a function that is called
//   whenever this extension is shut down (e.g. Crosswalk terminating). NULL
//   is fine.
// - context_create, filled with a pointer to a function that creates
//   an extension context (see comment below).

#if defined(_WIN32)
#define PUBLIC_EXPORT __declspec(dllexport)
#else
#define PUBLIC_EXPORT __attribute__((visibility("default")))
#endif

#if defined(__cplusplus)
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

EXTERN_C PUBLIC_EXPORT CXWalkExtension* xwalk_extension_init(
      int32_t api_version);

// A CXWalkExtension structure holds the global state for a extension.
// Due to the multithreaded way Crosswalk is written, one should not
// store mutable state there. That's the reason CXWalkExtensionContext
// exists: so each page context has its own state and there's no need
// to worry about race conditions while keeping state between contexts.
//
// The first two fields of a context (internal_data and api) should not
// be tampered with. The following fields, though, should be filled:
// - destroy, with a pointer to a function that will be called whenever
//   a particular context is about to be destroyed.
// - handle_message, with a pointer to a function that will be called
//   whenever a message arrives from the JavaScript side.
// - handle_sync_message, with a pointer to a function that will be
//   called whenever a synchronous message arrives, the reply should
//   be set before the function returns using
//   xwalk_extension_context_set_sync_reply().
//
// To post a message to the JavaScript side, one can simply call
// xwalk_extension_context_post_message(), defined below. Crosswalk will
// handle all the multithreading details so it is safe to call this
// whenever necessary.
static void xwalk_extension_context_post_message(
      CXWalkExtensionContext* context, const char* message) {
  assert(context);
  assert(context->api);
  assert(context->api->post_message);
  assert(message);

  context->api->post_message(context, message);
}

// To be used in the function passed as handle_sync_message.
static void xwalk_extension_context_set_sync_reply(
    CXWalkExtensionContext* context, const char* reply) {
  assert(context);
  assert(context->api);
  assert(context->api->set_sync_reply);
  assert(reply);

  context->api->set_sync_reply(context, reply);
}

#undef PUBLIC_EXPORT
#undef EXTERN_C

#endif  // INTERNAL_IMPLEMENTATION

#endif  // XWALK_EXTENSIONS_PUBLIC_XWALK_EXTENSION_PUBLIC_H_
