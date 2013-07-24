// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include "ipc/ipc_message_macros.h"

// Note: it is safe to use numbers after LastIPCMsgStart since that limit
// is not relevant for embedders. It is used only by a tool inside chrome/
// that we currently don't use.
// See also https://code.google.com/p/chromium/issues/detail?id=110911.
const int XWalkExtensionMsgStart = LastIPCMsgStart + 1;

#define IPC_MESSAGE_START XWalkExtensionMsgStart

IPC_MESSAGE_ROUTED2(XWalkViewHostMsg_PostMessage,  // NOLINT(*)
                    std::string /* target extension */,
                    std::string /* contents */)

IPC_MESSAGE_ROUTED2(XWalkViewMsg_PostMessage,  // NOLINT(*)
                    std::string /* source extension */,
                    std::string /* contents */)

IPC_MESSAGE_CONTROL2(XWalkViewMsg_RegisterExtension,  // NOLINT(*)
                    std::string /* extension */,
                    std::string /* JS API code for extension */)
