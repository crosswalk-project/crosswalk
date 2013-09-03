// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_SERVER_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_SERVER_H_

#include "base/values.h"
#include "ipc/ipc_channel_proxy.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_sender.h"
#include "xwalk/extensions/common/xwalk_extension_runner.h"

namespace xwalk {
namespace extensions {

class ExtensionServerMessageFilter;
class XWalkExtension;

// This class holds the Native context of Extensions. It can live in the Browser
// Process (for in-process extensions) or on the Extension Process. It
// communicates with its associated XWalkExtensionClient through an IPC channel.
class XWalkExtensionServer : public IPC::Listener, public IPC::Sender,
                             public XWalkExtensionRunner::Client {
 public:
  XWalkExtensionServer();
  virtual ~XWalkExtensionServer();

  // IPC::Listener Implementation.
  virtual bool OnMessageReceived(const IPC::Message& message);

  // IPC::Sender Implementation.
  virtual bool Send(IPC::Message* msg);

  void SetChannelProxy(IPC::ChannelProxy* channel);

  // FIXME(jeez): we should pass a scoped_ptr.
  bool RegisterExtension(XWalkExtension* extension);

 private:
  // Message Handlers
  void OnCreateInstance(int64_t instance_id, std::string name);
  void OnDestroyInstance(int64_t instance_id);
  void OnPostMessageToNative(int64_t instance_id, const base::ListValue& msg);
  void OnSendSyncMessageToNative(int64_t instance_id, const base::ListValue& msg,
      IPC::Message* ipc_reply);

  // XWalkExtensionRunner::Client implementation.
  virtual void HandleMessageFromNative(const XWalkExtensionRunner* runner,
                                        scoped_ptr<base::Value> msg) OVERRIDE;
  virtual void HandleReplyMessageFromNative(
      scoped_ptr<IPC::Message> ipc_reply, scoped_ptr<base::Value> msg) OVERRIDE;

  IPC::ChannelProxy* channel_;
  ExtensionServerMessageFilter* message_filter_;

  typedef std::map<std::string, XWalkExtension*> ExtensionMap;
  ExtensionMap extensions_;

  typedef std::map<int64_t, XWalkExtensionRunner*> RunnerMap;
  RunnerMap runners_;
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
