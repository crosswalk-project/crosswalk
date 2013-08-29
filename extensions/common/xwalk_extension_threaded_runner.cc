// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension_threaded_runner.h"

#include "base/bind.h"
#include "base/single_thread_task_runner.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"

namespace xwalk {
namespace extensions {

// This object is responsible for calling the client on behalf of the extension
// thread. When the threaded runner is destroyed, it detaches this object so
// pending tasks posted to client_task_runner are ignored gracefully.
class XWalkExtensionThreadedRunner::PostHelper {
 public:
  explicit PostHelper(XWalkExtensionThreadedRunner* runner)
      : runner_(runner) {}

  // Threaded runner calls this when is destroyed, so any pending tasks will not
  // call the client anymore. To be called in the thread controlling the
  // threaded runner.
  void Invalidate() {
    base::AutoLock lock(lock_);
    runner_ = NULL;
  }

  void PostMessageToClient(scoped_ptr<base::Value> msg) {
    base::AutoLock lock(lock_);
    if (!runner_)
      return;
    CHECK(runner_->client_task_runner_ == base::MessageLoopProxy::current());
    runner_->PostMessageToClient(msg.Pass());
  }

  void PostReplyMessageToClient(scoped_ptr<IPC::Message> ipc_reply,
                                scoped_ptr<base::Value> msg) {
    base::AutoLock lock(lock_);
    if (!runner_)
      return;
    CHECK(runner_->client_task_runner_ == base::MessageLoopProxy::current());
    runner_->PostReplyMessageToClient(ipc_reply.Pass(), msg.Pass());
  }

  // The helper will be destroyed when this function leaves and the scoped_ptr
  // goes out of scope. We post this function passing the internal helper object
  // in the client task runner so that is run after all pending post messages
  // from the context to the client.
  static void Destroy(scoped_ptr<PostHelper> helper) {}

 private:
  base::Lock lock_;
  XWalkExtensionThreadedRunner* runner_;
};

XWalkExtensionThreadedRunner::XWalkExtensionThreadedRunner(
    XWalkExtension* extension, Client* client,
    base::SingleThreadTaskRunner* client_task_runner)
    : XWalkExtensionRunner(extension->name(), client),
      extension_(extension),
      sync_message_event_(false, false),
      client_task_runner_(client_task_runner),
      helper_(new PostHelper(this)) {
  CHECK(client_task_runner_);
  std::string thread_name = "XWalk_ExtensionThread_" + extension_->name();
  thread_.reset(new base::Thread(thread_name.c_str()));
  thread_->Start();
  PostTaskToExtensionThread(
      FROM_HERE,
      base::Bind(&XWalkExtensionThreadedRunner::CreateContext,
                 base::Unretained(this)));
}

XWalkExtensionThreadedRunner::~XWalkExtensionThreadedRunner() {
  // Invalidate our helper poster. From now on calls to post will be ignored
  // since the client doesn't care about us anymore.
  helper_->Invalidate();

  // All Context related code should run in the thread.
  PostTaskToExtensionThread(
      FROM_HERE,
      base::Bind(&XWalkExtensionThreadedRunner::DestroyContext,
                 base::Unretained(this)));

  // Waiting on the browser thread is frowned upon since it might cause jank
  // (there's no way to know how much time the extension thread might be blocked
  // for).  However, it's not safe to destroy the extension context from a
  // thread other than the extension thread.  We temporarily turn off this
  // restriction, wait for the extension thread to be properly destroyed, and
  // enable the restriction again.
  //
  // FIXME(leandro): Find a way to properly destroy an extension context without
  // blocking the browser thread.
  base::ThreadRestrictions::ScopedAllowIO allow_io;
  thread_->Stop();
}

void XWalkExtensionThreadedRunner::HandleMessageFromClient(
    scoped_ptr<base::Value> msg) {
  PostTaskToExtensionThread(
      FROM_HERE,
      base::Bind(&XWalkExtensionThreadedRunner::CallHandleMessage,
                 base::Unretained(this),
                 base::Passed(&msg)));
}

void XWalkExtensionThreadedRunner::HandleSyncMessageFromClient(
    scoped_ptr<IPC::Message> ipc_reply, scoped_ptr<base::Value> msg) {
  PostTaskToExtensionThread(
      FROM_HERE,
      base::Bind(&XWalkExtensionThreadedRunner::CallHandleSyncMessage,
                 base::Unretained(this),
                 base::Passed(&ipc_reply),
                 base::Passed(&msg)));
}

bool XWalkExtensionThreadedRunner::CalledOnExtensionThread() const {
  return base::MessageLoop::current() == thread_->message_loop();
}

bool XWalkExtensionThreadedRunner::PostTaskToExtensionThread(
    const tracked_objects::Location& from_here,
    const base::Closure& task) {
  // FIXME(cmarcelo): Can we cache this message_loop_proxy?
  return thread_->message_loop_proxy()->PostTask(from_here, task);
}

void XWalkExtensionThreadedRunner::CreateContext() {
  CHECK(CalledOnExtensionThread());

  XWalkExtensionInstance* instance = extension_->CreateInstance(base::Bind(
      &XWalkExtensionThreadedRunner::PostMessageToClientTaskRunner,
      base::Unretained(this)));
  if (!instance) {
    VLOG(0) << "Could not create instance for extension '"
            << extension_->name() << "'. Destroying extension thread.";
    delete this;
    return;
  }

  context_.reset(instance);
}

void XWalkExtensionThreadedRunner::DestroyContext() {
  CHECK(CalledOnExtensionThread());
  context_.reset();

  // Trigger destruction of the helper object. We do at this point so that it
  // will be after any pending task posted by the extension thread.
  client_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&PostHelper::Destroy, base::Passed(&helper_)));
}

void XWalkExtensionThreadedRunner::CallHandleMessage(
    scoped_ptr<base::Value> msg) {
  CHECK(CalledOnExtensionThread());
  CHECK(context_);
  context_->HandleMessage(msg.Pass());
}

void XWalkExtensionThreadedRunner::CallHandleSyncMessage(
    scoped_ptr<IPC::Message> ipc_reply, scoped_ptr<base::Value> msg) {
  CHECK(CalledOnExtensionThread());

  scoped_ptr<base::Value> result_msg(
      context_->HandleSyncMessage(msg.Pass()));

  client_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&PostHelper::PostReplyMessageToClient,
                 base::Unretained(helper_.get()),
                 base::Passed(&ipc_reply),
                 base::Passed(&result_msg)));
}

void XWalkExtensionThreadedRunner::PostMessageToClientTaskRunner(
    scoped_ptr<base::Value> msg) {
  client_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&PostHelper::PostMessageToClient,
                 base::Unretained(helper_.get()),
                 base::Passed(&msg)));
}

}  // namespace extensions
}  // namespace xwalk
