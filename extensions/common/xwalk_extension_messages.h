// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include "ipc/ipc_message_macros.h"

#define IPC_MESSAGE_START ExtensionMsgStart

IPC_MESSAGE_ROUTED2(XWalkViewHostMsg_PostMessage,  // NOLINT(*)
                    std::string /* target extension */,
                    std::string /* contents */)

IPC_MESSAGE_ROUTED2(XWalkViewMsg_PostMessage,  // NOLINT(*)
                    std::string /* source extension */,
                    std::string /* contents */)

IPC_MESSAGE_CONTROL2(XWalkViewMsg_RegisterExtension,  // NOLINT(*)
                    std::string /* extension */,
                    std::string /* JS API code for extension */)
