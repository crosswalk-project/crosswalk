// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_external_instance.h"

#include <string>
#include "base/logging.h"
#include "xwalk/extensions/common/xwalk_external_extension.h"
#include "xwalk/extensions/common/xwalk_external_adapter.h"

namespace xwalk {
namespace extensions {

XWalkExternalInstance::XWalkExternalInstance(
    XWalkExternalExtension* extension, XW_Instance xw_instance)
    : xw_instance_(xw_instance),
      extension_(extension),
      instance_data_(NULL),
      is_handling_sync_msg_(false) {
  XWalkExternalAdapter::GetInstance()->RegisterInstance(this);
  XW_CreatedInstanceCallback callback = extension_->created_instance_callback_;
  if (callback)
    callback(xw_instance_);
}

XWalkExternalInstance::~XWalkExternalInstance() {
  XW_DestroyedInstanceCallback callback =
      extension_->destroyed_instance_callback_;
  if (callback)
    callback(xw_instance_);
  XWalkExternalAdapter::GetInstance()->UnregisterInstance(this);
}

void XWalkExternalInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  XW_HandleMessageCallback callback = extension_->handle_msg_callback_;
  if (!callback) {
    LOG(WARNING) << "Ignoring message sent for external extension '"
                 << extension_->name() << "' which doesn't support it.";
    return;
  }

  std::string string_msg;
  msg->GetAsString(&string_msg);
  callback(xw_instance_, string_msg.c_str());
}

void XWalkExternalInstance::HandleSyncMessage(scoped_ptr<base::Value> msg) {
  XW_HandleSyncMessageCallback callback = extension_->handle_sync_msg_callback_;
  if (!callback) {
    LOG(WARNING) << "Ignoring sync message sent for external extension '"
                 << extension_->name() << "' which doesn't support it.";
    return;
  }

  std::string string_msg;
  msg->GetAsString(&string_msg);

  callback(xw_instance_, string_msg.c_str());
}

void XWalkExternalInstance::CoreSetInstanceData(void* data) {
  instance_data_ = data;
}

void* XWalkExternalInstance::CoreGetInstanceData() {
  return instance_data_;
}

void XWalkExternalInstance::MessagingPostMessage(const char* msg) {
  PostMessageToJS(scoped_ptr<base::Value>(new base::StringValue(msg)));
}

void XWalkExternalInstance::SyncMessagingSetSyncReply(const char* reply) {
  SendSyncReplyToJS(scoped_ptr<base::Value>(new base::StringValue(reply)));
}

}  // namespace extensions
}  // namespace xwalk
