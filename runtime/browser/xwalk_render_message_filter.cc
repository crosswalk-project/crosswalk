// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_render_message_filter.h"

#include "xwalk/runtime/common/xwalk_common_messages.h"
#include "xwalk/runtime/browser/runtime_platform_util.h"

namespace xwalk {

XWalkRenderMessageFilter::XWalkRenderMessageFilter()
    : BrowserMessageFilter(ViewMsgStart) {
}

bool XWalkRenderMessageFilter::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkRenderMessageFilter, message)
    IPC_MESSAGE_HANDLER(ViewMsg_OpenLinkExternal, OnOpenLinkExternal)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void XWalkRenderMessageFilter::OnOpenLinkExternal(const GURL& url) {
  LOG(INFO) << "OpenLinkExternal: " << url.spec();
  platform_util::OpenExternal(url);
}

}  // namespace xwalk
