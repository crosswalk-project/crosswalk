// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included file, no traditional include guard.
#include "content/public/common/common_param_traits.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_platform_file.h"
#include "url/gurl.h"

// Singly-included section for enums and custom IPC traits.
#ifndef XWALK_RUNTIME_COMMON_XWALK_COMMON_MESSAGES_H_
#define XWALK_RUNTIME_COMMON_XWALK_COMMON_MESSAGES_H_

namespace IPC {

// TODO(upstream): - add enums and custom IPC traits here when needed.

}  // namespace IPC

#endif  // XWALK_RUNTIME_COMMON_XWALK_COMMON_MESSAGES_H_

#define IPC_MESSAGE_START ViewMsgStart

//-----------------------------------------------------------------------------
// RenderView messages
// These are messages sent from the browser to the renderer process.

IPC_MESSAGE_CONTROL3(ViewMsg_SetAccessWhiteList,  // NOLINT
                     GURL /* source */,
                     GURL /* dest */,
                     bool /* allow_subdomains */)

IPC_MESSAGE_CONTROL0(ViewMsg_EnableWarpMode)  // NOLINT
