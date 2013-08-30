// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_H_
#define XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_H_

// Crosswalk Extensions are modules of code loaded by Crosswalk runtime that
// allow extending its capabilities. The extension is expected to define a
// XW_Initialize() function as declared below, get the interfaces it need to
// use and register to whatever callbacks it needs, then return XW_OK.
//
// The Extension is represented by the type XW_Extension. Each extension
// loaded may be used multiple times for different pages, so to each execution
// there will be an associated XW_Instance. A reasonable analogy is that the
// XW_Extension represent a "class", and have concrete instances running.
//
// An interface is a struct with a set of functions, provided by Crosswalk,
// that allow the extension code to interact with the web content. Certain
// functions in an interface are used to register callbacks, so that Crosswalk
// can call the extension at specific situations.
//
// Crosswalk won't call an extension's XW_Initialize() multiple times in the
// same process.

#ifdef __cplusplus
extern "C" {
#endif

#if __GNUC__ >= 4
#define XW_EXPORT __attribute__ ((visibility("default")))
#elif defined(_MSC_VER)
#define XW_EXPORT __declspec(dllexport)
#endif

#include <stdint.h>


// XW_Extension is used to identify your extension when calling functions from
// the API. You should always use the XW_Extension received at XW_Initialize().
//
// XW_Instance is used to identify different web contents using your
// extension. Each time a new web content is created you can be notified
// registering the XW_CreatedInstanceCallback, that receives the new
// XW_Instance. When interacting with an Instance (for example to post a
// message), you should pass the corresponding XW_Instance.
//
// In both types the zero value is never used by Crosswalk, so can be used to
// initialize variables.
typedef int32_t XW_Extension;
typedef int32_t XW_Instance;

enum {
  XW_OK = 0,
  XW_ERROR = -1
};

// Returns a struct containing functions to be used by the extension. Those
// structs can be stored statically and used until the extension is unloaded.
// Extensions should use definitions like XW_CORE_INTERFACE, instead of using
// the versioned definition or the literal string. Returns NULL if the
// interface is not supported.
typedef const void* (*XW_GetInterface)(const char* interface_name);


typedef int32_t (*XW_Initialize_Func)(XW_Extension extension,
                                      XW_GetInterface get_interface);

// XW_Initialize is called after the extension code is loaded. The 'extension'
// value should be used in further calls that expect XW_Extension argument.
//
// The 'get_interface' function should be used to get access to functions that
// interact with the web content. It is only valid during the execution of the
// XW_Initialize() function.
//
// This function should return XW_OK when the extension was succesfully
// loaded, otherwise XW_ERROR.
XW_EXPORT int32_t XW_Initialize(XW_Extension extension,
                                XW_GetInterface get_interface);


//
// XW_CORE_INTERFACE: Basic functionality for Crosswalk Extensions. All
// extensions should use this interface to set at least their name.
//

#define XW_CORE_INTERFACE_1 "XW_CoreInterface_1"
#define XW_CORE_INTERFACE XW_CORE_INTERFACE_1

typedef void (*XW_CreatedInstanceCallback)(XW_Instance instance);
typedef void (*XW_DestroyedInstanceCallback)(XW_Instance instance);
typedef void (*XW_ShutdownCallback)(XW_Extension extension);

struct XW_CoreInterface_1 {
  // Set the name of the extension. It is used as the namespace for the
  // JavaScript code exposed by the extension. So extension named
  // 'my_extension', will expose its JavaScript functionality inside
  // the 'my_extension' namespace.
  //
  // This function should be called only during XW_Initialize().
  void (*SetExtensionName)(XW_Extension extension, const char* name);

  // Set the JavaScript code loaded in the web content when the extension is
  // used. This can be used together with the messaging mechanism to implement
  // a higher-level API that posts messages to extensions, see
  // XW_MESSAGING_INTERFACE below.
  //
  // The code will be executed inside a JS function context with the following
  // objects available:
  //
  // - exports: this object should be filled with properties and functions
  //            that will be exposed in the namespace associated with this
  //            extension.
  //
  // - extension.postMessage(): post a string message to the extension native
  //                            code. See below for details.
  // - extension.setMessageListener(): allow setting a callback that is called
  //                                   when the native code sends a message
  //                                   to JavaScript. Callback takes a string.
  //
  // This function should be called only during XW_Initialize().
  void (*SetJavaScriptAPI)(XW_Extension extension, const char* api);

  // Register callbacks that are called when an instance of this extension
  // is created or destroyed. Everytime a new web content is loaded, it will
  // get a new associated instance.
  //
  // This function should be called only during XW_Initialize().
  void (*RegisterInstanceCallbacks)(XW_Extension extension,
                                    XW_CreatedInstanceCallback created,
                                    XW_DestroyedInstanceCallback destroyed);

  // Register a callback to be executed when the extension will be unloaded.
  //
  // This function should be called only during XW_Initialize().
  void (*RegisterShutdownCallback)(XW_Extension extension,
                                   XW_ShutdownCallback shutdown_callback);

  // These two functions are conveniences used to associated arbitrary data
  // with a given XW_Instance. They can be used only with instances that were
  // created but not yet completely destroyed. GetInstanceData() can be used
  // during the destroyed instance callback. If not instance data was set,
  // getting it returns NULL.
  void (*SetInstanceData)(XW_Instance instance, void* data);
  void* (*GetInstanceData)(XW_Instance instance);
};

typedef struct XW_CoreInterface_1 XW_CoreInterface;


//
// XW_MESSAGING_INTERFACE: Exchange asynchronous messages with JavaScript
// code provided by extension.
//

#define XW_MESSAGING_INTERFACE_1 "XW_MessagingInterface_1"
#define XW_MESSAGING_INTERFACE XW_MESSAGING_INTERFACE_1

typedef void (*XW_HandleMessageCallback)(XW_Instance instance,
                                         const char* message);

struct XW_MessagingInterface_1 {
  // Register a callback to be called when the JavaScript code associated
  // with the extension posts a message. Note that the callback will be called
  // with the XW_Instance that posted the message as well as the message
  // contents.
  void (*Register)(XW_Extension extension,
                   XW_HandleMessageCallback handle_message);

  // Post a message to the web content associated with the instance. To
  // receive this message the extension's JavaScript code should set a
  // listener using extension.setMessageListener() function.
  //
  // This function is thread-safe and can be called until the instance is
  // destroyed.
  void (*PostMessage)(XW_Instance instance, const char* message);
};

typedef struct XW_MessagingInterface_1 XW_MessagingInterface;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_H_
