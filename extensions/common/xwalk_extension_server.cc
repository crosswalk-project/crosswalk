// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension_server.h"

#include "ipc/ipc_sender.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/common/xwalk_extension_threaded_runner.h"

namespace xwalk {
namespace extensions {

XWalkExtensionServer::XWalkExtensionServer(IPC::Sender* sender)
    : sender_(sender) {
}

XWalkExtensionServer::~XWalkExtensionServer() {
  //FIXME(jeez): clean up ALL OF OUR MAPS!!!!!

  // ExtensionMap::iterator it = extensions_.begin();
  // for (; it != extensions_.end(); ++it)
  //   delete it->second;
}

bool XWalkExtensionServer::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionServer, message)
    IPC_MESSAGE_HANDLER(XWalkExtensionServerMsg_CreateInstance,
        OnCreateInstance)
    IPC_MESSAGE_HANDLER(XWalkExtensionServerMsg_DestroyInstance,
        OnDestroyInstance)
    IPC_MESSAGE_HANDLER(XWalkExtensionServerMsg_PostMessageToNative,
        OnPostMessageToNative)
    IPC_MESSAGE_HANDLER_DELAY_REPLY(XWalkExtensionServerMsg_SendSyncMessageToNative,
        OnSendSyncMessageToNative)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void XWalkExtensionServer::OnCreateInstance(int64_t instance_id,
    std::string name) {
  ExtensionMap::const_iterator it = extensions_.find(name);

  if (it == extensions_.end()) {
    LOG(WARNING) << "Can't create instance of extension: " << name
        << ". Extension is not registered.";
    return;
  }

  XWalkExtensionRunner* runner = new XWalkExtensionThreadedRunner(
      it->second, this, base::MessageLoopProxy::current(), instance_id);

  runners_[instance_id] = runner;
}

void XWalkExtensionServer::OnPostMessageToNative(int64_t instance_id,
    const base::ListValue& msg) {
  RunnerMap::const_iterator it = runners_.find(instance_id);
  if (it == runners_.end()) {
    LOG(WARNING) << "Can't PostMessage to invalid Extension instance id: "
        << instance_id;
    return;
  }

  // The const_cast is needed to remove the only Value contained by the
  // ListValue (which is solely used as wrapper, since Value doesn't
  // have param traits for serialization) and we pass the ownership to to
  // HandleMessage. It is safe to do this because the |msg| won't be used
  // anywhere else when this function returns. Saves a DeepCopy(), which
  // can be costly depending on the size of Value.
  base::Value* value;
  const_cast<base::ListValue*>(&msg)->Remove(0, &value);
  (it->second)->PostMessageToNative(scoped_ptr<base::Value>(value));
}

bool XWalkExtensionServer::Send(IPC::Message* msg) {
  DCHECK(sender_);

  return sender_->Send(msg);
}

// FIXME(jeez): we should receive a scoped_ptr.
bool XWalkExtensionServer::RegisterExtension(XWalkExtension* extension) {
  if (extensions_.find(extension->name()) != extensions_.end()) {
    LOG(WARNING) << "Ignoring extension with name already registered: "
                 << extension->name();
    return false;
  }

  std::string name = extension->name();
  extensions_[name] = extension;
  return true;
}

void XWalkExtensionServer::HandleMessageFromNative(
    const XWalkExtensionRunner* runner, scoped_ptr<base::Value> msg) {
  base::ListValue list;
  list.Append(msg.release());

  Send(new XWalkExtensionClientMsg_PostMessageToJS(runner->instance_id(),
      list));
}

void XWalkExtensionServer::HandleReplyMessageFromNative(
      scoped_ptr<IPC::Message> ipc_reply, scoped_ptr<base::Value> msg) {
  base::ListValue result;
  result.Append(msg.release());

  IPC::WriteParam(ipc_reply.get(), result);
  Send(ipc_reply.release());
}

void XWalkExtensionServer::OnSendSyncMessageToNative(int64_t instance_id,
    const base::ListValue& msg, IPC::Message* ipc_reply) {
  RunnerMap::const_iterator it = runners_.find(instance_id);
  if (it == runners_.end()) {
    LOG(WARNING) << "Can't SendSyncMessage to invalid Extension instance id: "
        << instance_id;
    return;
  }

  base::Value* value;
  const_cast<base::ListValue*>(&msg)->Remove(0, &value);

  // We handle a pre-populated |ipc_reply| to the Instance, so it is up to the
  // it to decide when to reply. It is important to notice that the callee
  // on the renderer will remain blocked until the reply gets back.
  (it->second)->SendSyncMessageToNative(scoped_ptr<IPC::Message>(ipc_reply),
                                   scoped_ptr<base::Value>(value));
}

void XWalkExtensionServer::OnDestroyInstance(int64_t instance_id) {
  RunnerMap::iterator it = runners_.find(instance_id);
  if (it == runners_.end()) {
    LOG(WARNING) << "Can't destroy inexistent instance:" << instance_id;
    return;
  }

  delete it->second;
  runners_.erase(it);
}

}  // namespace extensions
}  // namespace xwalk

