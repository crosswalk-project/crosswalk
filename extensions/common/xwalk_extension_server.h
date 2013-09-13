// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_SERVER_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_SERVER_H_

#include <stdint.h>
#include <map>
#include <string>

#include "base/synchronization/cancellation_flag.h"
#include "base/values.h"
#include "ipc/ipc_channel_proxy.h"
#include "ipc/ipc_listener.h"
#include "xwalk/extensions/common/xwalk_extension_runner.h"

namespace base {
class FilePath;
}

namespace content {
class RenderProcessHost;
}

namespace IPC {
class Sender;
}

namespace xwalk {
namespace extensions {

class XWalkExtension;

// This class holds the Native context of Extensions. It can live in the Browser
// Process (for in-process extensions) or on the Extension Process. It
// communicates with its associated XWalkExtensionClient through an IPC channel.
class XWalkExtensionServer : public IPC::Listener,
                             public XWalkExtensionRunner::Client {
 public:
  XWalkExtensionServer();
  virtual ~XWalkExtensionServer();

  // IPC::Listener Implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void OnChannelConnected(int32 peer_pid) OVERRIDE;

  void Initialize(IPC::Sender* sender) { sender_ = sender; }
  bool Send(IPC::Message* msg);

  bool RegisterExtension(scoped_ptr<XWalkExtension> extension);
  void RegisterExtensionsInRenderProcess();

  void Invalidate();

 private:
  // Message Handlers
  void OnCreateInstance(int64_t instance_id, std::string name);
  void OnDestroyInstance(int64_t instance_id);
  void OnPostMessageToNative(int64_t instance_id, const base::ListValue& msg);
  void OnSendSyncMessageToNative(int64_t instance_id,
      const base::ListValue& msg, IPC::Message* ipc_reply);

  // XWalkExtensionRunner::Client implementation.
  virtual void HandleMessageFromNative(const XWalkExtensionRunner* runner,
                                        scoped_ptr<base::Value> msg) OVERRIDE;
  virtual void HandleReplyMessageFromNative(
      scoped_ptr<IPC::Message> ipc_reply, scoped_ptr<base::Value> msg) OVERRIDE;

  IPC::Sender* sender_;

  typedef std::map<std::string, XWalkExtension*> ExtensionMap;
  ExtensionMap extensions_;

  typedef std::map<int64_t, XWalkExtensionRunner*> RunnerMap;
  RunnerMap runners_;

  base::CancellationFlag sender_cancellation_flag_;
};

void RegisterExternalExtensionsInDirectory(
    XWalkExtensionServer* server, const base::FilePath& dir);

bool ValidateExtensionNameForTesting(const std::string& extension_name);

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_SERVER_H_
