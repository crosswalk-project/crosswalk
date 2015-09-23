// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/win/xwalk_dotnet_instance.h"

#include <vector>

#include "base/logging.h"
#include "xwalk/extensions/common/win/xwalk_dotnet_extension.h"

namespace {

typedef void(*post_message_callback_func)(void*, const std::string&);
typedef void(*set_sync_reply_callback_func)(void*, const std::string&);
typedef void (CALLBACK* set_post_message_callback)(
  void*, post_message_callback_func);
typedef void (CALLBACK* set_set_sync_reply_callback)(
  void*, set_sync_reply_callback_func);
typedef void* (CALLBACK* CreateDotNetInstance)(void*, void*);
typedef void* (CALLBACK* HandleMessageType)(void*, void*, const std::string&);
typedef void* (CALLBACK* HandleSyncMessageType)(
  void*, void*, const std::string&);

}  // namespace

namespace xwalk {
namespace extensions {

XWalkDotNetInstance::XWalkDotNetInstance(
  XWalkDotNetExtension* extension) : extension_(extension) {
  CreateDotNetInstance create_instance_ptr =
      (CreateDotNetInstance)GetProcAddress(extension_->GetBridgeHandle(),
      "CreateDotNetInstance");
  instance_dotnet_ = create_instance_ptr(extension_->GetBridge(), this);

  set_post_message_callback set_post_message_callback_ptr =
      (set_post_message_callback)GetProcAddress(extension_->GetBridgeHandle(),
      "set_post_message_callback");
  set_post_message_callback_ptr(
      extension_->GetBridge(), &PostMessageToJSCallback);

  set_set_sync_reply_callback set_set_sync_reply_callback_ptr =
      (set_set_sync_reply_callback)GetProcAddress(extension_->GetBridgeHandle(),
      "set_set_sync_reply_callback");
  set_set_sync_reply_callback_ptr(extension_->GetBridge(), &SetSyncReply);
}

XWalkDotNetInstance::~XWalkDotNetInstance() {
}

void XWalkDotNetInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  if (!instance_dotnet_)
    return;
  std::string string_msg;
  if (!msg->GetAsString(&string_msg)) {
    LOG(WARNING) << "Failed to retrieve the message's value.";
    return;
  }
  HandleMessageType handle_message_ptr =
      (HandleMessageType)GetProcAddress(extension_->GetBridgeHandle(),
      "HandleMessage");
  handle_message_ptr(extension_->GetBridge(), instance_dotnet_, string_msg);
}

void XWalkDotNetInstance::HandleSyncMessage(scoped_ptr<base::Value> msg) {
  if (!instance_dotnet_)
    return;
  std::string string_msg;
  if (!msg->GetAsString(&string_msg)) {
    LOG(WARNING) << "Failed to retrieve the message's value.";
    return;
  }

  HandleSyncMessageType handle_message_ptr =
      (HandleSyncMessageType)GetProcAddress(extension_->GetBridgeHandle(),
      "HandleSyncMessage");
  handle_message_ptr(extension_->GetBridge(), instance_dotnet_, string_msg);
}

void XWalkDotNetInstance::PostMessageToJSCallback(
    void* instance, const std::string& message) {
  if (!instance)
    return;
    XWalkDotNetInstance* inst =
        reinterpret_cast<XWalkDotNetInstance*>(instance);
    inst->PostMessageToJS(scoped_ptr<base::Value>(
        new base::StringValue(message)));
}

void XWalkDotNetInstance::SetSyncReply(
    void* instance, const std::string& message) {
  if (!instance)
    return;
  XWalkDotNetInstance* inst =
      reinterpret_cast<XWalkDotNetInstance*>(instance);
  inst->SendSyncReplyToJS(scoped_ptr<base::Value>(
      new base::StringValue(message)));
}

}  // namespace extensions
}  // namespace xwalk
