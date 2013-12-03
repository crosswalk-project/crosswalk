// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension.h"

#include "base/logging.h"

namespace xwalk {
namespace extensions {

XWalkExtension::XWalkExtension()
  : permissions_delegate_(NULL) {}

XWalkExtension::~XWalkExtension() {}

const base::ListValue& XWalkExtension::entry_points() const {
  return entry_points_;
}

bool XWalkExtension::CheckAPIAccessControl(std::string app_id,
    std::string api_name) {
  if (!permissions_delegate_) {
      return true;
  }
  return permissions_delegate_->CheckAPIAccessControl(name_, app_id, api_name);
}

XWalkExtensionInstance::XWalkExtensionInstance() {}

XWalkExtensionInstance::~XWalkExtensionInstance() {}

void XWalkExtensionInstance::SetPostMessageCallback(
    const PostMessageCallback& callback) {
  post_message_ = callback;
}

void XWalkExtensionInstance::SetSendSyncReplyCallback(
    const SendSyncReplyCallback& callback) {
  send_sync_reply_ = callback;
}

void XWalkExtensionInstance::HandleSyncMessage(
    scoped_ptr<base::Value> msg) {
  LOG(FATAL) << "Sending sync message to extension which doesn't support it!";
}

}  // namespace extensions
}  // namespace xwalk
