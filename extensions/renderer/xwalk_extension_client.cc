// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_client.h"

#include "base/values.h"
#include "ipc/ipc_sender.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/renderer/xwalk_extension_module.h"
#include "xwalk/extensions/renderer/xwalk_module_system.h"

namespace xwalk {
namespace extensions {

XWalkExtensionClient::XWalkExtensionClient(IPC::Sender* sender)
    : sender_(sender),
      next_instance_id_(0) {
}

bool XWalkExtensionClient::Send(IPC::Message* msg) {
  DCHECK(sender_);

  return sender_->Send(msg);
}

XWalkRemoteExtensionRunner* XWalkExtensionClient::CreateRunner(
    const std::string& extension_name,
    XWalkRemoteExtensionRunner::Client* client) {
  if (!Send(new XWalkExtensionServerMsg_CreateInstance(next_instance_id_,
    extension_name))) {
    return 0;
  }

  XWalkRemoteExtensionRunner* runner = new XWalkRemoteExtensionRunner(client,
      this, next_instance_id_);

  runners_[next_instance_id_] = runner;
  next_instance_id_++;

  return runner;
}

bool XWalkExtensionClient::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionClient, message)
    IPC_MESSAGE_HANDLER(XWalkExtensionClientMsg_PostMessageToJS,
        OnPostMessageToJS)
    IPC_MESSAGE_HANDLER(XWalkExtensionClientMsg_RegisterExtension,
        OnRegisterExtension)
    IPC_MESSAGE_HANDLER(XWalkExtensionClientMsg_InstanceDestroyed,
        OnInstanceDestroyed)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void XWalkExtensionClient::OnPostMessageToJS(int64_t instance_id,
    const base::ListValue& msg) {
  RunnerMap::const_iterator it = runners_.find(instance_id);
  if (it == runners_.end() || !it->second) {
    LOG(WARNING) << "Can't PostMessage to invalid Extension instance id: "
        << instance_id;
    return;
  }

  const base::Value* value;
  msg.Get(0, &value);
  (it->second)->PostMessageToJS(*value);
}

void XWalkExtensionClient::DestroyInstance(int64_t instance_id) {
  RunnerMap::iterator it = runners_.find(instance_id);
  if (it == runners_.end() || !it->second) {
    LOG(WARNING) << "Can't Destroy invalid instance id: " << instance_id;
    return;
  }

  Send(new XWalkExtensionServerMsg_DestroyInstance(instance_id));

  delete it->second;
  it->second = 0;
}

void XWalkExtensionClient::OnInstanceDestroyed(int64_t instance_id) {
  RunnerMap::iterator it = runners_.find(instance_id);
  if (it == runners_.end())
    return;

  // The runner should be invalid (null) at this point since it should have
  // been destroyed in XWalkExtensionClient::DestroyInstance(). If we ever
  // find out that the Server can kill the Instance and only after let us know
  // then we should modify this to if(it->second) { delete it->second; }
  DCHECK(!it->second);

  // Take it out from the valid runners map.
  runners_.erase(it);
}

void XWalkExtensionClient::CreateRunnersForModuleSystem(XWalkModuleSystem*
    module_system) {
  // FIXME(cmarcelo): Load extensions sorted by name so parent comes first, so
  // that we can safely register all them.
  ExtensionAPIMap::const_iterator it = extension_apis_.begin();
  for (; it != extension_apis_.end(); ++it) {
    if (it->second.empty())
      continue;
    scoped_ptr<XWalkExtensionModule> module(
        new XWalkExtensionModule(module_system, it->first, it->second));
    XWalkRemoteExtensionRunner* runner = CreateRunner(it->first, module.get());
    module->set_runner(runner);
    module_system->RegisterExtensionModule(module.Pass());
  }
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

  base::Value* reply;
  wrapped_reply->Remove(0, &reply);
  return scoped_ptr<base::Value>(reply);
}

}  // namespace extensions
}  // namespace xwalk
