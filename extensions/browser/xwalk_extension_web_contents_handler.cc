// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/extensions/browser/xwalk_extension_web_contents_handler.h"

#include "base/bind.h"
#include "base/threading/thread.h"
#include "cameo/extensions/browser/xwalk_extension.h"
#include "cameo/extensions/common/xwalk_extension_messages.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(
    xwalk::extensions::XWalkExtensionWebContentsHandler);

namespace tracked_objects {
class Location;
}

namespace xwalk {
namespace extensions {

// This class wraps an XWalkExtension::Context so it runs in its own Extension
// thread. It also provides |post_message| callback for the Context.
class XWalkExtensionRunner {
 public:
  XWalkExtensionRunner(XWalkExtension* extension,
                       IPC::Sender* sender,
                       int32_t render_view_id)
      : extension_(extension),
        sender_(sender),
        render_view_id_(render_view_id) {
    std::string thread_name = "XWalk_ExtensionThread_" + extension_->name();
    thread_.reset(new base::Thread(thread_name.c_str()));
    thread_->Start();
    PostTaskToExtensionThread(
        FROM_HERE, base::Bind(&XWalkExtensionRunner::CreateContext,
                              base::Unretained(this)));
  }

  ~XWalkExtensionRunner() {
    // All Context related code should run in the thread.
    PostTaskToExtensionThread(
        FROM_HERE, base::Bind(&XWalkExtensionRunner::DestroyContext,
                              base::Unretained(this)));

    // Note: this will block until threads message loop process the task above.
    thread_->Stop();
  }

  void HandleMessage(const std::string& msg) {
    PostTaskToExtensionThread(
        FROM_HERE, base::Bind(&XWalkExtension::Context::HandleMessage,
                              base::Unretained(context_.get()),
                              msg));
  }

  void PostMessage(const std::string& msg) {
    sender_->Send(
        new XWalkViewMsg_PostMessage(render_view_id_, extension_->name(), msg));
  }

 private:
  bool PostTaskToExtensionThread(const tracked_objects::Location& from_here,
                                 const base::Closure& task) {
    // FIXME(cmarcelo): Can we cache this message_loop_proxy?
    return thread_->message_loop_proxy()->PostTask(from_here, task);
  }

  void CreateContext() {
    CHECK(CalledOnExtensionThread());

    XWalkExtension::Context* context = extension_->CreateContext(base::Bind(
          &XWalkExtensionRunner::PostMessage, base::Unretained(this)));
    if (!context) {
      VLOG(0) << "Could not create context for extension \"" <<
            extension_->name() << "\". Destroying extension thread.";
      delete this;
      return;
    }

    context_.reset(context);
  }

  void DestroyContext() {
    CHECK(CalledOnExtensionThread());
    context_.reset();
  }

  bool CalledOnExtensionThread() {
    return MessageLoop::current() == thread_->message_loop();
  }

  scoped_ptr<XWalkExtension::Context> context_;
  scoped_ptr<base::Thread> thread_;
  XWalkExtension* extension_;
  IPC::Sender* sender_;
  int32_t render_view_id_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionRunner);
};

XWalkExtensionWebContentsHandler::XWalkExtensionWebContentsHandler(
    content::WebContents* contents)
    : WebContentsObserver(contents) {
}

XWalkExtensionWebContentsHandler::~XWalkExtensionWebContentsHandler() {
  RunnerMap::iterator it = runners_.begin();
  for (; it != runners_.end(); ++it)
    delete it->second;
}

void XWalkExtensionWebContentsHandler::AttachExtension(
    XWalkExtension* extension) {
  runners_[extension->name()] =
      new XWalkExtensionRunner(extension,
                               this,
                               web_contents()->GetRoutingID());
}

bool XWalkExtensionWebContentsHandler::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionWebContentsHandler, message)
    IPC_MESSAGE_HANDLER(XWalkViewHostMsg_PostMessage, OnPostMessage)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkExtensionWebContentsHandler::OnPostMessage(
    const std::string& extension_name, const std::string& msg) {
  RunnerMap::iterator it = runners_.find(extension_name);
  if (it != runners_.end())
    it->second->HandleMessage(msg);
  else
    LOG(WARNING) << "Couldn't handle message for unregistered extension '"
                 << extension_name << "'";
}

}  // namespace extensions
}  // namespace xwalk
