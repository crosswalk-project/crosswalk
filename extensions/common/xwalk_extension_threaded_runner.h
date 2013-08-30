// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_THREADED_RUNNER_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_THREADED_RUNNER_H_

#include <string>
#include "base/callback_forward.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/synchronization/waitable_event.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_runner.h"

namespace base {
class SingleThreadTaskRunner;
class Thread;
}

namespace tracked_objects {
class Location;
}

namespace xwalk {
namespace extensions {

// Creates and runs an extension context in a thread. All operations from the
// extension context will be called in the separated thread.
//
// The given task runner correspond to the thread that will handle the calls
// to Client. After XWalkExtensionThreadedRunner is deleted, the client will
// not be called anymore.
class XWalkExtensionThreadedRunner : public XWalkExtensionRunner {
 public:
  XWalkExtensionThreadedRunner(
      XWalkExtension* extension, Client* client,
      base::SingleThreadTaskRunner* client_task_runner);
  virtual ~XWalkExtensionThreadedRunner();

 private:
  // XWalkExtensionRunner implementation.
  virtual void HandleMessageFromClient(scoped_ptr<base::Value> msg) OVERRIDE;
  virtual void HandleSyncMessageFromClient(
      scoped_ptr<IPC::Message> ipc_reply, scoped_ptr<base::Value> msg) OVERRIDE;

  bool CalledOnExtensionThread() const;
  bool PostTaskToExtensionThread(const tracked_objects::Location& from_here,
                                 const base::Closure& task);
  void CreateContext();
  void DestroyContext();

  void CallHandleMessage(scoped_ptr<base::Value> msg);
  void CallHandleSyncMessage(scoped_ptr<IPC::Message> ipc_reply,
                             scoped_ptr<base::Value> msg);

  void PostMessageToClientTaskRunner(scoped_ptr<base::Value> msg);

  scoped_ptr<XWalkExtensionInstance> context_;
  scoped_ptr<base::Thread> thread_;
  XWalkExtension* extension_;

  base::WaitableEvent sync_message_event_;
  base::SingleThreadTaskRunner* client_task_runner_;

  class PostHelper;
  scoped_ptr<PostHelper> helper_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionThreadedRunner);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_THREADED_RUNNER_H_
