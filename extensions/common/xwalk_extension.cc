// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension.h"

#include "base/logging.h"

namespace xwalk {
namespace extensions {

XWalkExtension::XWalkExtension() {}

XWalkExtension::~XWalkExtension() {}

XWalkExtensionInstance::XWalkExtensionInstance() {}

void XWalkExtensionInstance::SetPostMessageCallback(const
    XWalkExtension::PostMessageCallback& post_message) {
  post_message_ = post_message;
}

void XWalkExtensionInstance::SetSendSyncReplyCallback(
    const XWalkExtension::SendSyncReplyCallback& callback) {
  send_sync_reply_ = callback;
}

XWalkExtensionInstance::~XWalkExtensionInstance() {}

void XWalkExtensionInstance::HandleSyncMessage(
    scoped_ptr<base::Value> msg) {
  LOG(FATAL) << "Sending sync message to extension which doesn't support it!";
}

}  // namespace extensions
}  // namespace xwalk
