// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_client.h"

#include "base/values.h"
#include "base/stl_util.h"
#include "ipc/ipc_sender.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"

namespace xwalk {
namespace extensions {

XWalkExtensionClient::XWalkExtensionClient()
    : sender_(0),
      next_instance_id_(1) {  // Zero is never used for a valid instance.
}

XWalkExtensionClient::~XWalkExtensionClient() {
  STLDeleteValues(&extension_apis_);
}

bool XWalkExtensionClient::Send(IPC::Message* msg) {
  DCHECK(sender_);

  return sender_->Send(msg);
}

int64_t XWalkExtensionClient::CreateInstance(
    const std::string& extension_name,
    InstanceHandler* handler) {
  CHECK(handler);
  if (!Send(new XWalkExtensionServerMsg_CreateInstance(next_instance_id_,
                                                       extension_name))) {
    return 0;
  }
  handlers_[next_instance_id_] = handler;
  return next_instance_id_++;
}

bool XWalkExtensionClient::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionClient, message)
    IPC_MESSAGE_HANDLER(XWalkExtensionClientMsg_PostMessageToJS,
        OnPostMessageToJS)
    IPC_MESSAGE_HANDLER(XWalkExtensionClientMsg_InstanceDestroyed,
        OnInstanceDestroyed)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

XWalkExtensionClient::ExtensionCodePoints::ExtensionCodePoints() {
}

XWalkExtensionClient::ExtensionCodePoints::~ExtensionCodePoints() {
}

void XWalkExtensionClient::OnPostMessageToJS(int64_t instance_id,
                                             const base::ListValue& msg) {
  HandlerMap::const_iterator it = handlers_.find(instance_id);
  if (it == handlers_.end()) {
    LOG(WARNING) << "Can't PostMessage to invalid Extension instance id: "
                 << instance_id;
    return;
  }

  // See comment in DestroyInstance() about two step destruction.
  if (!it->second)
    return;

  const base::Value* value;
  msg.Get(0, &value);
  it->second->HandleMessageFromNative(*value);
}

void XWalkExtensionClient::DestroyInstance(int64_t instance_id) {
  HandlerMap::iterator it = handlers_.find(instance_id);
  if (it == handlers_.end() || !it->second) {
    LOG(WARNING) << "Can't Destroy invalid instance id: " << instance_id;
    return;
  }
  Send(new XWalkExtensionServerMsg_DestroyInstance(instance_id));

  // Destruction happens in two steps, first we nullify the handler in our map,
  // to indicate that destruction message was sent. If we get a new message from
  // this instance, we can silently ignore. Later, we get a confirmation message
  // from the server, only then we remove the entry from the map.
  it->second = NULL;
}

void XWalkExtensionClient::OnInstanceDestroyed(int64_t instance_id) {
  HandlerMap::iterator it = handlers_.find(instance_id);
  if (it == handlers_.end()) {
    LOG(WARNING) << "Got InstanceDestroyed msg for invalid instance id: "
                 << instance_id;
    return;
  }

  // Second part of the two step destruction. See DestroyInstance() for details.
  // The system currently assumes that we always control the destruction of
  // instances.
  DCHECK(!it->second);
  handlers_.erase(it);
}

namespace {

// Regular base::Value doesn't have param traits, so can't be passed as is
// through IPC. We wrap it in a base::ListValue that have traits before
// exchanging.
//
// Implementing param traits for base::Value is not a viable option at the
// moment (would require fork base::Value and create a new empty type).
scoped_ptr<base::ListValue> WrapValueInList(scoped_ptr<base::Value> value) {
  if (!value)
    return scoped_ptr<base::ListValue>();
  scoped_ptr<base::ListValue> list_value(new base::ListValue);
  list_value->Append(value.release());
  return list_value.Pass();
}

}  // namespace

void XWalkExtensionClient::PostMessageToNative(int64_t instance_id,
    scoped_ptr<base::Value> msg) {
  scoped_ptr<base::ListValue> list_msg = WrapValueInList(msg.Pass());
  Send(new XWalkExtensionServerMsg_PostMessageToNative(instance_id, *list_msg));
}

scoped_ptr<base::Value> XWalkExtensionClient::SendSyncMessageToNative(
    int64_t instance_id, scoped_ptr<base::Value> msg) {
  scoped_ptr<base::ListValue> wrapped_msg = WrapValueInList(msg.Pass());
  base::ListValue* wrapped_reply = new base::ListValue;
  Send(new XWalkExtensionServerMsg_SendSyncMessageToNative(instance_id,
      *wrapped_msg, wrapped_reply));

  scoped_ptr<base::Value> reply;
  wrapped_reply->Remove(0, &reply);
  return reply.Pass();
}

void XWalkExtensionClient::Initialize(IPC::Sender* sender) {
  sender_ = sender;

  std::vector<XWalkExtensionServerMsg_ExtensionRegisterParams> extensions;
  Send(new XWalkExtensionServerMsg_GetExtensions(&extensions));

  if (extensions.empty())
    return;

  std::vector<XWalkExtensionServerMsg_ExtensionRegisterParams>::iterator it =
      extensions.begin();
  for (; it != extensions.end(); ++it) {
    ExtensionCodePoints* codepoint = new ExtensionCodePoints;
    codepoint->api = (*it).js_api;

    codepoint->entry_points = (*it).entry_points;

    std::string name = (*it).name;
    extension_apis_[name] = codepoint;
  }
}

}  // namespace extensions
}  // namespace xwalk
