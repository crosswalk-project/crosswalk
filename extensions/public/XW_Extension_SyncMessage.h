// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_SYNCMESSAGE_H_
#define XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_SYNCMESSAGE_H_

// NOTE: This file and interfaces marked as internal are not considered stable
// and can be modified in incompatible ways between Crosswalk versions.

#ifndef XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_H_
#error "You should include XW_Extension.h before this file"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// XW_INTERNAL_SYNC_MESSAGING_INTERFACE: allow JavaScript code to send a
// synchronous message to extension code and block until response is
// available. The response is made available by calling the SetSyncReply
// function, that can be done from outside the context of the SyncMessage
// handler.
//

#define XW_INTERNAL_SYNC_MESSAGING_INTERFACE_1 \
  "XW_InternalSyncMessagingInterface_1"
#define XW_INTERNAL_SYNC_MESSAGING_INTERFACE \
  XW_INTERNAL_SYNC_MESSAGING_INTERFACE_1

typedef void (*XW_HandleSyncMessageCallback)(XW_Instance instance,
                                             const char* message);

struct XW_Internal_SyncMessagingInterface_1 {
  void (*Register)(XW_Extension extension,
                   XW_HandleSyncMessageCallback handle_sync_message);
  void (*SetSyncReply)(XW_Instance instance, const char* reply);
};

typedef struct XW_Internal_SyncMessagingInterface_1
    XW_Internal_SyncMessagingInterface;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // XWALK_EXTENSIONS_PUBLIC_XW_EXTENSION_SYNCMESSAGE_H_
