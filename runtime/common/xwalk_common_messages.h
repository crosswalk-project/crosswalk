// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included file, no traditional include guard.
#include <string>
#include <vector>

#include "content/public/common/common_param_traits.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_platform_file.h"
#include "url/gurl.h"
#include "xwalk/application/browser/application_security_policy.h"

// Singly-included section for enums and custom IPC traits.
#ifndef XWALK_RUNTIME_COMMON_XWALK_COMMON_MESSAGES_H_
#define XWALK_RUNTIME_COMMON_XWALK_COMMON_MESSAGES_H_

namespace IPC {

// TODO(upstream): - add enums and custom IPC traits here when needed.

}  // namespace IPC

#endif  // XWALK_RUNTIME_COMMON_XWALK_COMMON_MESSAGES_H_

#define IPC_MESSAGE_START ViewMsgStart

IPC_ENUM_TRAITS(xwalk::application::ApplicationSecurityPolicy::SecurityMode)
//-----------------------------------------------------------------------------
// RenderView messages
// These are messages sent from the browser to the renderer process.

IPC_MESSAGE_CONTROL3(ViewMsg_SetAccessWhiteList,  // NOLINT
                     GURL /* source */,
                     GURL /* dest */,
                     bool /* allow_subdomains */)

IPC_MESSAGE_CONTROL2(ViewMsg_EnableSecurityMode,    // NOLINT
                     GURL /* application url */,
                     xwalk::application::ApplicationSecurityPolicy::SecurityMode
                     /* security mode */)

IPC_MESSAGE_ROUTED1(ViewMsg_HWKeyPressed, int /*keycode*/)  // NOLINT

// These are messages sent from the renderer to the browser process.
IPC_MESSAGE_CONTROL1(ViewMsg_OpenLinkExternal,  // NOLINT
                     GURL /* target link */)

#if defined(ENABLE_PEPPER_CDMS)
// Returns whether any internal plugin supporting |mime_type| is registered and
// enabled. Does not determine whether the plugin can actually be instantiated
// (e.g. whether it has all its dependencies).
// When the returned *|is_available| is true, |additional_param_names| and
// |additional_param_values| contain the name-value pairs, if any, specified
// for the *first* non-disabled plugin found that is registered for |mime_type|.
IPC_SYNC_MESSAGE_CONTROL1_3(  // NOLINT
    XwalkViewHostMsg_IsInternalPluginAvailableForMimeType,
    std::string /* mime_type */,
    bool /* is_available */,
    std::vector<base::string16> /* additional_param_names */,
    std::vector<base::string16> /* additional_param_values */)
#endif  // XWALK_RUNTIME_COMMON_XWALK_COMMON_MESSAGES_H_
