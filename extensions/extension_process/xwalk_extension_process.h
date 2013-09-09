// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_EXTENSION_PROCESS_XWALK_EXTENSION_PROCESS_H_
#define XWALK_EXTENSIONS_EXTENSION_PROCESS_XWALK_EXTENSION_PROCESS_H_

#include "base/message_loop.h"
#include "base/run_loop.h"
#include "base/values.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "ipc/ipc_listener.h"
#include "xwalk/extensions/common/xwalk_extension_runner.h"

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

class XWalkExtensionProcess : public IPC::Listener,
                              public XWalkExtensionRunner::Client {
 public:
  XWalkExtensionProcess();
  virtual ~XWalkExtensionProcess();

  void Run();

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

  base::MessageLoop main_loop_;
  base::RunLoop run_loop_;
  base::WaitableEvent shutdown_event_;
  base::Thread io_thread_;
  scoped_ptr<IPC::SyncChannel> browser_process_channel_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionProcess);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_EXTENSION_PROCESS_XWALK_EXTENSION_PROCESS_H_
