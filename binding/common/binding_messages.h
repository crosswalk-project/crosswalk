// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_BINDING_COMMON_BINDING_MESSAGES_H_
#define XWALK_BINDING_COMMON_BINDING_MESSAGES_H_

#include <string>
#include <vector>

#include "content/common/content_param_traits.h"
#include "content/public/common/common_param_traits.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_message_macros.h"

#define BindingMsgStart (LastIPCMsgStart + 1)
#define IPC_MESSAGE_START BindingMsgStart

IPC_SYNC_MESSAGE_ROUTED1_1(ViewHostMsg_OpenBindingChannel,
                           GURL /* page_url */,
                           IPC::ChannelHandle /* channel_handle */)

IPC_SYNC_MESSAGE_CONTROL0_1(BindingMsg_GenerateRouteID,
                            int /* id */)

IPC_SYNC_MESSAGE_CONTROL0_1(BindingMsg_BindAPIs,
                            bool /* success */)

IPC_SYNC_MESSAGE_CONTROL2_0(BindingHostMsg_SetException,
                            int, /* route id */
                            content::NPVariant_Param /* exception */)

IPC_MESSAGE_CONTROL0(BindingHostMsg_ShuttingDown)

IPC_MESSAGE_CONTROL3(BindingProcessMsg_CreateChannel,
                     int /* routing_id */,
                     GURL /* page_url */,
                     std::vector<std::string>)

IPC_MESSAGE_CONTROL0(BindingProcessMsg_ShuttingDown)

IPC_MESSAGE_CONTROL2(BindingProcessHostMsg_ChannelCreated,
                     int /* renderer_id */,
                     IPC::ChannelHandle /* channel_handle */)

#endif  // XWALK_BINDING_COMMON_BINDING_MESSAGES_H_
