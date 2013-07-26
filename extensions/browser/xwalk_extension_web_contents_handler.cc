// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_web_contents_handler.h"

#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/common/xwalk_extension_threaded_runner.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(
    xwalk::extensions::XWalkExtensionWebContentsHandler);

namespace xwalk {
namespace extensions {

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
      new XWalkExtensionThreadedRunner(extension, this);
}

void XWalkExtensionWebContentsHandler::HandleMessageFromContext(
    const XWalkExtensionRunner* runner, scoped_ptr<base::Value> msg) {
  base::ListValue list;
  list.Append(msg.release());

  Send(new XWalkViewMsg_PostMessage(web_contents()->GetRoutingID(),
                                    runner->extension_name(), list));
}

bool XWalkExtensionWebContentsHandler::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionWebContentsHandler, message)
    IPC_MESSAGE_HANDLER(XWalkViewHostMsg_PostMessage, OnPostMessage)
    IPC_MESSAGE_HANDLER(XWalkViewHostMsg_SendSyncMessage, OnSendSyncMessage)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkExtensionWebContentsHandler::OnPostMessage(
    const std::string& extension_name, const base::ListValue& msg) {
  RunnerMap::iterator it = runners_.find(extension_name);
  if (it == runners_.end()) {
    LOG(WARNING) << "Couldn't handle message for unregistered extension '"
                 << extension_name << "'";
    return;
  }

  // The const_cast is need because we remove the only Value contained
  // by the ListValue (which is solely used as wrapper, since Value doesn't
  // have param traits for serialization) and we pass the ownership to to
  // HandleMessage. It is safe to do this because the |msg| won't be used
  // anywhere else when this function returns. This will save a DeepCopy(),
  // which can be costly depending on the size of Value.
  base::Value* value;
  const_cast<base::ListValue*>(&msg)->Remove(0, &value);
  it->second->PostMessageToContext(scoped_ptr<base::Value>(value));
}

void XWalkExtensionWebContentsHandler::OnSendSyncMessage(
    const std::string& extension_name, const std::string& msg,
    std::string* result) {
  RunnerMap::iterator it = runners_.find(extension_name);
  if (it != runners_.end())
    *result = it->second->SendSyncMessageToContext(msg);
  else
    LOG(WARNING) << "Couldn't handle sync message for unregistered extension '"
                 << extension_name << "'";
}

}  // namespace extensions
}  // namespace xwalk
