// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/binding/browser/browser_binding_helper.h"

#include "base/bind.h"
#include "base/logging.h"
#include "content/common/child_process.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "content/common/npobject_proxy.h"
#include "xwalk/binding/browser/binding_process_host.h"
#include "xwalk/binding/browser/binding_policy.h"
#include "xwalk/binding/common/binding_messages.h"

namespace xwalk {

BrowserBindingHelper::BrowserBindingHelper(content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      delay_reply_msg_(NULL) {
}

BrowserBindingHelper::~BrowserBindingHelper() {
  if (delay_reply_msg_)
    Send(delay_reply_msg_);
}

void BrowserBindingHelper::DidCreateBindingChannel(
    const IPC::ChannelHandle& channel_handle) {
  if (delay_reply_msg_) {
    ViewHostMsg_OpenBindingChannel::WriteReplyParams(delay_reply_msg_,
                                                     channel_handle);
    Send(delay_reply_msg_);
    delay_reply_msg_ = NULL;
  }
}

bool BrowserBindingHelper::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(BrowserBindingHelper, message)
    IPC_MESSAGE_HANDLER_DELAY_REPLY(ViewHostMsg_OpenBindingChannel,
                                    OnOpenBindingChannel)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void BrowserBindingHelper::OnOpenBindingChannel(const GURL& url,
                                                IPC::Message* reply_msg) {
  // Determine which APIs can be used in the frame.
  std::vector<std::string> features;
  features = BindingPolicy::GetService()->GetFeatures(url);
  if (!features.size() || delay_reply_msg_) {
    Send(reply_msg);
    return;
  }

  // FIXME: The callback should not be issued after this object is deleted.
  delay_reply_msg_ = reply_msg;
  content::BrowserThread::PostTask(
      content::BrowserThread::IO,
      FROM_HERE,
      base::Bind(&BrowserBindingHelper::CreateBindingChannel,
                 base::Unretained(this),
                 url,
                 features));
}

void BrowserBindingHelper::CreateBindingChannel(
    const GURL& url,
    const std::vector<std::string> features) {
  // Start binding process if not yet started.
  BindingProcessHost* host = BindingProcessHost::Get();

  // Requet binding process to create a binding channel for renderer
  host->CreateBindingChannel(routing_id(), url, features,
      base::Bind(&BrowserBindingHelper::DidCreateBindingChannel,
          base::Unretained(this)));
}

}  // namespace xwalk
