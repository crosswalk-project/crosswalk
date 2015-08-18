// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_MESSAGE_2_H_
#define XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_MESSAGE_2_H_

#ifndef XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_H_
#error "You should include XW_Extension.h before this file"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define XW_MESSAGING_INTERFACE_2 "XW_MessagingInterface_2"

typedef void (*XW_HandleBinaryMessageCallback)(XW_Instance instance,
                                               const char* message,
                                               const size_t size);

struct XW_MessagingInterface_2 {
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

  // Register a callback to be called when the JavaScript code associated
  // with the extension posts a binary message (ArrayBuffer object).
  // Note that the callback will be called with the XW_Instance that posted
  // the message as well as the message contents.
  void (*RegisterBinaryMesssageCallback)(
      XW_Extension extension,
      XW_HandleBinaryMessageCallback handle_message);

  // Post a binary message to the web content associated with the instance. To
  // receive this message the extension's JavaScript code should set a
  // listener using extension.setMessageListener() function.
  // The JavaScript message listener function would receive the binary message
  // in an ArrayBuffer object.
  //
  // This function is thread-safe and can be called until the instance is
  // destroyed.
  void (*PostBinaryMessage)(XW_Instance instance,
                            const char* message, size_t size);
};

typedef struct XW_MessagingInterface_2 XW_MessagingInterface2;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_MESSAGE_2_H_
