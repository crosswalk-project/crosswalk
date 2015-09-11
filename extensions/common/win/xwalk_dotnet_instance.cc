
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/win/xwalk_dotnet_instance.h"

#include "xwalk/extensions/common/win/xwalk_dotnet_bridge.h"
#include "xwalk/extensions/common/win/xwalk_dotnet_extension.h"

#include <string>
#include <vector>

namespace xwalk {
namespace extensions {

XWalkDotNetInstance::XWalkDotNetInstance(
  XWalkDotNetExtension* extension) : extension_(extension) {
  instance_dotnet_ = extension_->GetBridge()->CreateInstance(this);
  extension_->GetBridge()->set_post_message_callback(&PostMessageToJSCallback);
  extension_->GetBridge()->set_set_sync_reply_callback(&SetSyncReply);
}

XWalkDotNetInstance::~XWalkDotNetInstance() {
}

void XWalkDotNetInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  if (!instance_dotnet_)
    return;
  std::string string_msg;
  if (msg->GetAsString(&string_msg)) {
    extension_->GetBridge()->HandleMessage(instance_dotnet_, string_msg);
  } else {
    LOG(WARNING) << "Failed to retrieve the message's value.";
  }
}

void XWalkDotNetInstance::HandleSyncMessage(scoped_ptr<base::Value> msg) {
  if (!instance_dotnet_)
    return;
  std::string string_msg;
  if (msg->GetAsString(&string_msg)) {
    extension_->GetBridge()->HandleSyncMessage(instance_dotnet_, string_msg);
  } else {
    LOG(WARNING) << "Failed to retrieve the message's value.";
  }
}

void XWalkDotNetInstance::PostMessageToJSCallback(
  void* instance, const std::string& message) {
  if (instance) {
    XWalkDotNetInstance* inst =
      reinterpret_cast<XWalkDotNetInstance*>(instance);
    inst->PostMessageToJS(
      scoped_ptr<base::Value>(
      new base::StringValue(message)));
  }
}

void XWalkDotNetInstance::SetSyncReply(
  void* instance, const std::string& message) {
  if (instance) {
    XWalkDotNetInstance* inst =
      reinterpret_cast<XWalkDotNetInstance*>(instance);
    inst->SendSyncReplyToJS(
      scoped_ptr<base::Value>(
      new base::StringValue(message)));
  }
}

}  // namespace extensions
}  // namespace xwalk
