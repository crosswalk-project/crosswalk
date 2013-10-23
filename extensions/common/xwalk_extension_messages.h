// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>
#include <string>
#include "base/values.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_message_macros.h"

// Note: it is safe to use numbers after LastIPCMsgStart since that limit
// is not relevant for embedders. It is used only by a tool inside chrome/
// that we currently don't use.
// See also https://code.google.com/p/chromium/issues/detail?id=110911.
enum {
  XWalkExtensionMsgStart = LastIPCMsgStart + 1,
  XWalkExtensionClientServerMsgStart
};

#define IPC_MESSAGE_START XWalkExtensionMsgStart

IPC_MESSAGE_CONTROL1(XWalkExtensionProcessMsg_RegisterExtensions,  // NOLINT(*)
                     base::FilePath /* extensions path */)

IPC_MESSAGE_CONTROL1(XWalkExtensionProcessHostMsg_RenderProcessChannelCreated, // NOLINT(*)
                     IPC::ChannelHandle /* channel id */)

IPC_MESSAGE_CONTROL0(XWalkExtensionProcessHostMsg_ExtensionsRegistered) // NOLINT(*)

IPC_MESSAGE_CONTROL1(XWalkViewMsg_ExtensionProcessChannelCreated, // NOLINT(*)
                     IPC::ChannelHandle /* channel id */)


// We use a separated message class for Client<->Server communication
// to ease filtering.
#undef IPC_MESSAGE_START
#define IPC_MESSAGE_START XWalkExtensionClientServerMsgStart

IPC_MESSAGE_CONTROL2(XWalkExtensionServerMsg_CreateInstance,  // NOLINT(*)
                     int64_t /* instance id */,
                     std::string /* extension name */)

IPC_MESSAGE_CONTROL2(XWalkExtensionServerMsg_PostMessageToNative,  // NOLINT(*)
                     int64_t /* instance id */,
                     base::ListValue /* contents */)

IPC_MESSAGE_CONTROL2(XWalkExtensionClientMsg_PostMessageToJS,  // NOLINT(*)
                     int64_t /* instance id */,
                     base::ListValue /* contents */)

IPC_SYNC_MESSAGE_CONTROL2_1(XWalkExtensionServerMsg_SendSyncMessageToNative,  // NOLINT(*)
                            int64_t /* instance id */,
                            base::ListValue /* input contents */,
                            base::ListValue /* output contents */)

IPC_SYNC_MESSAGE_CONTROL0_1(XWalkExtensionServerMsg_GetExtensionsToRegister,  // NOLINT(*)
                            base::ListValue /* output contents */)

IPC_MESSAGE_CONTROL1(XWalkExtensionServerMsg_DestroyInstance,  // NOLINT(*)
                     int64_t /* instance id */)

IPC_MESSAGE_CONTROL1(XWalkExtensionClientMsg_InstanceDestroyed,  // NOLINT(*)
                     int64_t /* instance id */)
