// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/binding/renderer/renderer_binding_helper.h"

#include "content/common/child_process.h"
#include "content/common/npobject_stub.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebDocument.h"
#include "xwalk/binding/common/binding_messages.h"
#include "xwalk/binding/renderer/binding_channel_host.h"

namespace xwalk {

RendererBindingHelper::RendererBindingHelper(content::RenderView* render_view)
    : content::RenderViewObserver(render_view),
      content::RenderViewObserverTracker<RendererBindingHelper>(render_view) {
}

void RendererBindingHelper::DidClearWindowObject(WebKit::WebFrame* frame) {
  GURL url(frame->document().url());
  IPC::ChannelHandle channel_handle;
  Send(new ViewHostMsg_OpenBindingChannel(routing_id(), url, &channel_handle));
  if (channel_handle.name.empty())
    return;

  // Create channel host
  channel_host_ = BindingChannelHost::GetBindingChannelHost(channel_handle,
      content::ChildProcess::current()->io_message_loop_proxy());
  if (!channel_host_.get()) {
    LOG(ERROR) << "Couldn't get BindingChannelHost";
    return;
  }
  if (!channel_host_->GetNPObjectListenerForRoute(0))
    new content::NPObjectStub(frame->windowObject(), channel_host_.get(),
                              0, routing_id(), url);

  bool result = false;
  channel_host_->Send(new BindingMsg_BindAPIs(&result));
  if (!result) {
    // FIXME(zliang7) cleanup channel
  }
}

}  // namespace xwalk
