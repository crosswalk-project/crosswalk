// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_render_message_filter.h"

#if defined(OS_ANDROID)
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge_base.h"
#include "xwalk/runtime/browser/android/xwalk_contents_io_thread_client.h"
#include "xwalk/runtime/common/android/xwalk_render_view_messages.h"
#endif

#include "xwalk/runtime/common/xwalk_common_messages.h"
#include "xwalk/runtime/browser/runtime_platform_util.h"

using content::BrowserThread;

namespace xwalk {

XWalkRenderMessageFilter::XWalkRenderMessageFilter()
    : BrowserMessageFilter(ViewMsgStart) {
}

#if defined(OS_ANDROID)
XWalkRenderMessageFilter::XWalkRenderMessageFilter(int process_id)
    : BrowserMessageFilter(AndroidWebViewMsgStart),
      process_id_(process_id) {
}

void XWalkRenderMessageFilter::OverrideThreadForMessage(
    const IPC::Message& message,
    BrowserThread::ID* thread) {
  if (message.type() == XWalkViewHostMsg_ShouldOverrideUrlLoading::ID) {
    *thread = BrowserThread::UI;
  }
}
#endif

bool XWalkRenderMessageFilter::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkRenderMessageFilter, message)
    IPC_MESSAGE_HANDLER(ViewMsg_OpenLinkExternal, OnOpenLinkExternal)
#if defined(OS_ANDROID)
    IPC_MESSAGE_HANDLER(XWalkViewHostMsg_SubFrameCreated, OnSubFrameCreated)
    IPC_MESSAGE_HANDLER(XWalkViewHostMsg_ShouldOverrideUrlLoading,
                        OnShouldOverrideUrlLoading)
#endif
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void XWalkRenderMessageFilter::OnOpenLinkExternal(const GURL& url) {
  LOG(INFO) << "OpenLinkExternal: " << url.spec();
  platform_util::OpenExternal(url);
}

#if defined(OS_ANDROID)
void XWalkRenderMessageFilter::OnSubFrameCreated(
    int parent_render_frame_id, int child_render_frame_id) {
  XWalkContentsIoThreadClient::SubFrameCreated(
      process_id_, parent_render_frame_id, child_render_frame_id);
}

void XWalkRenderMessageFilter::OnShouldOverrideUrlLoading(
    int render_frame_id,
    const base::string16& url,
    bool has_user_gesture,
    bool is_redirect,
    bool is_main_frame,
    bool* ignore_navigation) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  *ignore_navigation = false;
  XWalkContentsClientBridgeBase* client =
      XWalkContentsClientBridgeBase::FromRenderFrameID(process_id_, render_frame_id);
  if (client) {
    *ignore_navigation = client->ShouldOverrideUrlLoading(
        url, has_user_gesture, is_redirect, is_main_frame);
  } else {
    LOG(WARNING) << "Failed to find the associated render view host for url: "
                 << url;
  }
}
#endif

}  // namespace xwalk
