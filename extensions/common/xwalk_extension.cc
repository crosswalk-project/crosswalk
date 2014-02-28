// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension.h"

#include <string.h>

#include "base/logging.h"

namespace xwalk {
namespace extensions {

bool XWalkExtension::PermissionsDelegate::CheckAPIAccessControl(
    const std::string& extension_name, const std::string& api_name) {
  return false;
}

bool XWalkExtension::PermissionsDelegate::RegisterPermissions(
    const std::string& extension_name, const std::string& perm_table) {
  return false;
}

XWalkExtension::XWalkExtension() : permissions_delegate_(NULL) {}

XWalkExtension::~XWalkExtension() {}

bool XWalkExtension::Initialize() {
  return false;
}

const base::ListValue& XWalkExtension::entry_points() const {
  return entry_points_;
}

bool XWalkExtension::CheckAPIAccessControl(const char* api_name) const {
  if (!permissions_delegate_)
    return false;

  return permissions_delegate_->CheckAPIAccessControl(name(), api_name);
}

bool XWalkExtension::RegisterPermissions(const char* perm_table) const {
  if (!permissions_delegate_)
    return false;

  return permissions_delegate_->RegisterPermissions(name(), perm_table);
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
