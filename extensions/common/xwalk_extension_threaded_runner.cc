// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension_threaded_runner.h"

#include "base/bind.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"

namespace xwalk {
namespace extensions {

XWalkExtensionThreadedRunner::XWalkExtensionThreadedRunner(
    XWalkExtension* extension, Client* client)
    : XWalkExtensionRunner(extension->name(), client),
      extension_(extension),
      sync_message_event_(false, false) {
  std::string thread_name = "XWalk_ExtensionThread_" + extension_->name();
  thread_.reset(new base::Thread(thread_name.c_str()));
  thread_->Start();
  PostTaskToExtensionThread(
      FROM_HERE,
      base::Bind(&XWalkExtensionThreadedRunner::CreateContext,
                 base::Unretained(this)));
}

XWalkExtensionThreadedRunner::~XWalkExtensionThreadedRunner() {
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

scoped_ptr<base::Value>
XWalkExtensionThreadedRunner::HandleSyncMessageFromClient(
    scoped_ptr<base::Value> msg) {
  base::Value* reply;
  PostTaskToExtensionThread(
      FROM_HERE,
      base::Bind(&XWalkExtensionThreadedRunner::CallHandleSyncMessage,
                 base::Unretained(this), base::Passed(&msg), &reply));
  sync_message_event_.Wait();
  return scoped_ptr<base::Value>(reply);
}

bool XWalkExtensionThreadedRunner::CalledOnExtensionThread() const {
  return MessageLoop::current() == thread_->message_loop();
}

bool XWalkExtensionThreadedRunner::PostTaskToExtensionThread(
    const tracked_objects::Location& from_here,
    const base::Closure& task) {
  // FIXME(cmarcelo): Can we cache this message_loop_proxy?
  return thread_->message_loop_proxy()->PostTask(from_here, task);
}

void XWalkExtensionThreadedRunner::CreateContext() {
  CHECK(CalledOnExtensionThread());

  XWalkExtension::Context* context = extension_->CreateContext(base::Bind(
      &XWalkExtensionThreadedRunner::PostMessageToClient,
      base::Unretained(this)));
  if (!context) {
    VLOG(0) << "Could not create context for extension '"
            << extension_->name() << "'. Destroying extension thread.";
    delete this;
    return;
  }

  context_.reset(context);
}

void XWalkExtensionThreadedRunner::DestroyContext() {
  CHECK(CalledOnExtensionThread());
  context_.reset();
}

void XWalkExtensionThreadedRunner::CallHandleMessage(
    scoped_ptr<base::Value> msg) {
  CHECK(CalledOnExtensionThread());
  CHECK(context_);
  context_->HandleMessage(msg.Pass());
}

void XWalkExtensionThreadedRunner::CallHandleSyncMessage(
    scoped_ptr<base::Value> msg, base::Value** reply) {
  CHECK(CalledOnExtensionThread());
  *reply = context_->HandleSyncMessage(msg.Pass()).release();
  sync_message_event_.Signal();
}

}  // namespace extensions
}  // namespace xwalk
