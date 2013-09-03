// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_client.h"

#include "base/values.h"
#include "ipc/ipc_sync_channel.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/renderer/xwalk_extension_module.h"
#include "xwalk/extensions/renderer/xwalk_module_system.h"

namespace xwalk {
namespace extensions {

XWalkExtensionClient::XWalkExtensionClient(IPC::ChannelProxy* channel)
    : channel_(channel),
      next_instance_id_(0) {
}

bool XWalkExtensionClient::Send(IPC::Message* msg) {
  DCHECK(channel_);

  return channel_->Send(msg);
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
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void XWalkExtensionClient::OnPostMessageToJS(int64_t instance_id,
    const base::ListValue& msg) {
  RunnerMap::const_iterator it = runners_.find(instance_id);
  if (it == runners_.end()) {
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
  if (it == runners_.end()) {
    LOG(WARNING) << "Can't Destroy invalid instance id: " << instance_id;
    return;
  }

  Send(new XWalkExtensionServerMsg_DestroyInstance(instance_id));

  // FIXME(jeez): should we wait a reply msg from server before deleting this?
  delete it->second;

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

}  // namespace extensions
}  // namespace xwalk
