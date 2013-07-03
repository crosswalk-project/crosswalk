// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/extensions/browser/cameo_extension_web_contents_handler.h"

#include "base/bind.h"
#include "base/threading/thread.h"
#include "cameo/extensions/browser/cameo_extension.h"
#include "cameo/extensions/common/cameo_extension_messages.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(
    cameo::extensions::CameoExtensionWebContentsHandler);

namespace tracked_objects {
class Location;
}

namespace cameo {
namespace extensions {

// This class wraps an CameoExtension::Context so it runs in its own Extension
// thread. It also provides |post_message| callback for the Context.
class CameoExtensionRunner {
 public:
  CameoExtensionRunner(CameoExtension* extension,
                       IPC::Sender* sender,
                       int32_t render_view_id)
      : extension_(extension),
        sender_(sender),
        render_view_id_(render_view_id) {
    std::string thread_name = "Cameo_ExtensionThread_" + extension_->name();
    thread_.reset(new base::Thread(thread_name.c_str()));
    thread_->Start();
    PostTaskToExtensionThread(
        FROM_HERE, base::Bind(&CameoExtensionRunner::CreateContext,
                              base::Unretained(this)));
  }

  ~CameoExtensionRunner() {
    // All Context related code should run in the thread.
    PostTaskToExtensionThread(
        FROM_HERE, base::Bind(&CameoExtensionRunner::DestroyContext,
                              base::Unretained(this)));

    // Note: this will block until threads message loop process the task above.
    thread_->Stop();
  }

  void HandleMessage(const std::string& msg) {
    PostTaskToExtensionThread(
        FROM_HERE, base::Bind(&CameoExtension::Context::HandleMessage,
                              base::Unretained(context_.get()),
                              msg));
  }

  void PostMessage(const std::string& msg) {
    sender_->Send(
        new CameoViewMsg_PostMessage(render_view_id_, extension_->name(), msg));
  }

 private:
  bool PostTaskToExtensionThread(const tracked_objects::Location& from_here,
                                 const base::Closure& task) {
    // FIXME(cmarcelo): Can we cache this message_loop_proxy?
    return thread_->message_loop_proxy()->PostTask(from_here, task);
  }

  void CreateContext() {
    CHECK(CalledOnExtensionThread());
    context_.reset(extension_->CreateContext(
        base::Bind(&CameoExtensionRunner::PostMessage,
                   base::Unretained(this))));
  }

  void DestroyContext() {
    CHECK(CalledOnExtensionThread());
    context_.reset();
  }

  bool CalledOnExtensionThread() {
    return MessageLoop::current() == thread_->message_loop();
  }

  scoped_ptr<CameoExtension::Context> context_;
  scoped_ptr<base::Thread> thread_;
  CameoExtension* extension_;
  IPC::Sender* sender_;
  int32_t render_view_id_;

  DISALLOW_COPY_AND_ASSIGN(CameoExtensionRunner);
};

CameoExtensionWebContentsHandler::CameoExtensionWebContentsHandler(
    content::WebContents* contents)
    : WebContentsObserver(contents) {
}

CameoExtensionWebContentsHandler::~CameoExtensionWebContentsHandler() {
  RunnerMap::iterator it = runners_.begin();
  for (; it != runners_.end(); ++it)
    delete it->second;
}

void CameoExtensionWebContentsHandler::AttachExtension(
    CameoExtension* extension) {
  runners_[extension->name()] =
      new CameoExtensionRunner(extension,
                               this,
                               web_contents()->GetRoutingID());
}

bool CameoExtensionWebContentsHandler::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(CameoExtensionWebContentsHandler, message)
    IPC_MESSAGE_HANDLER(CameoViewHostMsg_PostMessage, OnPostMessage)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void CameoExtensionWebContentsHandler::OnPostMessage(
    const std::string& extension_name, const std::string& msg) {
  RunnerMap::iterator it = runners_.find(extension_name);
  if (it != runners_.end())
    it->second->HandleMessage(msg);
  else
    LOG(WARNING) << "Couldn't handle message for unregistered extension '"
                 << extension_name << "'";
}

}  // namespace extensions
}  // namespace cameo
