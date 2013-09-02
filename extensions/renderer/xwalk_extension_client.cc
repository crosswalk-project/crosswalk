// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_client.h"

#include "base/values.h"
#include "ipc/ipc_sync_channel.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"

namespace xwalk {
namespace extensions {

XWalkExtensionClient::XWalkExtensionClient(IPC::ChannelProxy* channel)
    : channel_(channel),
      next_instance_id_(0) {
}

bool XWalkExtensionClient::Send(IPC::Message* msg) {
  DCHECK(channel_);

  return channel_->Send(msg);
}

// FIXME(jeez): this should be only (XWalkRemoteExtensionRunner::Client*)
XWalkRemoteExtensionRunner* XWalkExtensionClient::CreateRunner(
    XWalkExtensionRenderViewHandler* handler, int64_t frame_id,
    const std::string& extension_name,
    XWalkRemoteExtensionRunner::Client* client) {
  if (!Send(new XWalkExtensionServerMsg_CreateInstance(next_instance_id_,
    extension_name))) {
    return 0;
  }

  XWalkRemoteExtensionRunner* runner =
      new XWalkRemoteExtensionRunner(handler, frame_id, extension_name, client,
          this, next_instance_id_);

  runners_[next_instance_id_] = runner;
  next_instance_id_++;

  return runner;
}

bool XWalkExtensionClient::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionClient, message)
    IPC_MESSAGE_HANDLER(XWalkExtensionClientMsg_PostMessageToJS,
        OnPostMessageToJS)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void XWalkExtensionClient::OnPostMessageToJS(int64_t instance_id,
    const base::ListValue& msg) {
  RunnerMap::const_iterator it = runners_.find(instance_id);
  if (it == runners_.end()) {
    LOG(WARNING) << "Can't PostMessage to invalid Extension instance id: "
        << instance_id;
    return;
  }

  const base::Value* value;
  msg.Get(0, &value);
  (it->second)->PostMessageToJS(*value);
}

}  // namespace extensions
}  // namespace xwalk
