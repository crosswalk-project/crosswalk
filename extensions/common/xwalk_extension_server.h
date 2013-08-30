// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_SERVER_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_SERVER_H_

#include "base/values.h"
#include "ipc/ipc_channel_proxy.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_sender.h"

namespace xwalk {
namespace extensions {

class ExtensionServerMessageFilter;

// This class holds the Native context of Extensions. It can live in the Browser
// Process (for in-process extensions) or on the Extension Process. It
// communicates with its associated XWalkExtensionClient through an IPC channel.
class XWalkExtensionServer : public IPC::Listener, public IPC::Sender {
 public:
  XWalkExtensionServer(IPC::ChannelProxy* channel);
  virtual ~XWalkExtensionServer() {}

  // IPC::Listener Implementation.
  virtual bool OnMessageReceived(const IPC::Message& message);

  // IPC::Sender Implementation.
  virtual bool Send(IPC::Message* msg);

 private:
  void OnCreateInstance(int64_t instance_id, std::string name);
  void OnPostMessageToNative(int64_t instance_id, const base::ListValue& msg);

  IPC::ChannelProxy* channel_;
  ExtensionServerMessageFilter* message_filter_;
};

class ExtensionServerMessageFilter : public IPC::ChannelProxy::MessageFilter {
 public:
  ExtensionServerMessageFilter(XWalkExtensionServer* server);
  virtual ~ExtensionServerMessageFilter() {}

  virtual bool OnMessageReceived(const IPC::Message& message);

 private:
  XWalkExtensionServer* server_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_SERVER_H_
