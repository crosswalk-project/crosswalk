// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>
#include <string>

#include "base/memory/shared_memory.h"
#include "base/values.h"
#include "ipc/ipc_message_macros.h"
#include "xwalk/application/common/application.h"
#include "xwalk/application/common/manifest.h"

// Use Chrome extension message slot.
const int ApplicationMsgStart = ExtensionMsgStart;

#define IPC_MESSAGE_START ApplicationMsgStart

// Ask the main document if it is ready to be suspended. This is sent
// when main document is considered idle.
IPC_MESSAGE_CONTROL1(ApplicationMsg_ShouldSuspend,
                     int /* sequence_id**/)

// If we complete a round of ShouldSuspend->ShouldSuspendAck messages without
// the main document becoming active again, we are ready to unload. This
// message tells the main document to dispatch the suspend event.
IPC_MESSAGE_CONTROL0(ApplicationMsg_Suspend)

// The runtime process changes its mind about suspending the main document.
IPC_MESSAGE_CONTROL0(ApplicationMsg_CancelSuspend)

// Response to ApplicationMsg_ShouldSuspend.
IPC_MESSAGE_CONTROL1(ApplicationHostMsg_ShouldSuspendAck,
                     int /* sequence_id**/)

// Response to ApplicationMsg_Suspend.
IPC_MESSAGE_CONTROL0(ApplicationHostMsg_SuspendAck)
