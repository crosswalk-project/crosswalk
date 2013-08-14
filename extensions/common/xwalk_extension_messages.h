// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>
#include <string>
#include "base/values.h"
#include "ipc/ipc_message_macros.h"

// Note: it is safe to use numbers after LastIPCMsgStart since that limit
// is not relevant for embedders. It is used only by a tool inside chrome/
// that we currently don't use.
// See also https://code.google.com/p/chromium/issues/detail?id=110911.
const int XWalkExtensionMsgStart = LastIPCMsgStart + 1;

#define IPC_MESSAGE_START XWalkExtensionMsgStart

IPC_MESSAGE_ROUTED3(XWalkViewHostMsg_PostMessage,  // NOLINT(*)
                    int64_t /* frame id */,
                    std::string /* target extension */,
                    base::ListValue /* contents */)

IPC_MESSAGE_ROUTED3(XWalkViewMsg_PostMessage,  // NOLINT(*)
                    int64_t /* frame id */,
                    std::string /* source extension */,
                    base::ListValue /* contents */)

IPC_MESSAGE_CONTROL2(XWalkViewMsg_RegisterExtension,  // NOLINT(*)
                    std::string /* extension */,
                    std::string /* JS API code for extension */)

IPC_SYNC_MESSAGE_ROUTED3_1(XWalkViewHostMsg_SendSyncMessage,  // NOLINT(*)
                           int64_t /* frame id */,
                           std::string /* target extension */,
                           base::ListValue /* input contents */,
                           base::ListValue /* output contents */)

IPC_MESSAGE_ROUTED1(XWalkViewHostMsg_DidCreateScriptContext,  // NOLINT(*)
                    int64_t /* frame id */)

IPC_MESSAGE_ROUTED1(XWalkViewHostMsg_WillReleaseScriptContext,  // NOLINT(*)
                    int64_t /* frame id */)
