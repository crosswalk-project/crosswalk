// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_EXTENSION_PROCESS_XWALK_EXTENSION_PROCESS_H_
#define XWALK_EXTENSIONS_EXTENSION_PROCESS_XWALK_EXTENSION_PROCESS_H_

#include "base/values.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "ipc/ipc_listener.h"
#include "xwalk/extensions/common/xwalk_extension_runner.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"

namespace base {
class FilePath;
}

namespace IPC {
class SyncChannel;
}

namespace xwalk {
namespace extensions {

class XWalkExtension;
class XWalkExtensionRunner;

class DummySender;

// This class represents the Extension Process itself.
// It not only represents the extension side of the browser <->
// extension process communication channel, but also the extension side
// of the extension <-> render process channel.
// It will be responsible for handling the native side (instances) of
// External extensions through its XWalkExtensionServer.
class XWalkExtensionProcess : public IPC::Listener,
                              public XWalkExtensionRunner::Client {
 public:
  XWalkExtensionProcess();
  virtual ~XWalkExtensionProcess();

 private:
  // IPC::Listener implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  // XWalkExtensionRunner::Client implementation.
  virtual void HandleMessageFromNative(
      const XWalkExtensionRunner* runner, scoped_ptr<base::Value> msg) OVERRIDE;
  virtual void HandleReplyMessageFromNative(
      scoped_ptr<IPC::Message> ipc_reply, scoped_ptr<base::Value> msg) OVERRIDE;

  // Handlers for IPC messages from XWalkExtensionProcessHost.
  void OnRegisterExtensions(const base::FilePath& extension_path);

  void CreateChannel();

  base::WaitableEvent shutdown_event_;
  base::Thread io_thread_;
  scoped_ptr<IPC::SyncChannel> browser_process_channel_;
  XWalkExtensionServer extensions_server_;

  // FIXME(jeez): Remove this.
  scoped_ptr<DummySender> dummy_sender_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionProcess);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_EXTENSION_PROCESS_XWALK_EXTENSION_PROCESS_H_
