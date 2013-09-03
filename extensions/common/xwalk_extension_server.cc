// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension_server.h"

#include "xwalk/extensions/common/xwalk_extension_messages.h"

namespace xwalk {
namespace extensions {

XWalkExtensionServer::XWalkExtensionServer(IPC::ChannelProxy* channel)
    : channel_(channel) {
  // The filter is owned by the IPC channel, but a reference is kept for
  // explicitly removing it from the channel in case we get deleted but the
  // channel continues to be used.
  message_filter_ = new ExtensionServerMessageFilter(this);
  channel_->AddFilter(message_filter_);
}

bool XWalkExtensionServer::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionServer, message)
    IPC_MESSAGE_HANDLER(XWalkExtensionServerMsg_CreateInstance,
        OnCreateInstance)
    IPC_MESSAGE_HANDLER(XWalkExtensionServerMsg_PostMessageToNative,
        OnPostMessageToNative)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void XWalkExtensionServer::OnCreateInstance(int64_t instance_id,
    std::string name) {
  // FIXME(jeez): remove this!!! Only for testing purpose!!
  LOG(WARNING) << "ExtensionServer CreateInstance id: " << instance_id << " ExtensionName: " << name;
}

void XWalkExtensionServer::OnPostMessageToNative(int64_t instance_id,
    const base::ListValue& msg) {
  // FIXME(jeez): remove this!!! Only for testing purpose!!
  LOG(WARNING) << "\n\nExtensionServer PostMessageToNative!";
}

bool XWalkExtensionServer::Send(IPC::Message* msg) {
  DCHECK(channel_);

  return channel_->Send(msg);
}

ExtensionServerMessageFilter::ExtensionServerMessageFilter(
    XWalkExtensionServer* server)
    : server_(server) {
}

bool ExtensionServerMessageFilter::OnMessageReceived(const IPC::Message& msg) {
  return server_->OnMessageReceived(msg);
}

}  // namespace extensions
}  // namespace xwalk

