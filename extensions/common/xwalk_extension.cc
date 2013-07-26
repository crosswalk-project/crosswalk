// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension.h"

#include "base/logging.h"

namespace xwalk {
namespace extensions {

XWalkExtension::XWalkExtension() {}

XWalkExtension::~XWalkExtension() {}

XWalkExtension::Context::Context(const PostMessageCallback& post_message)
    : post_message_(post_message) {
}

XWalkExtension::Context::~Context() {}

std::string XWalkExtension::Context::HandleSyncMessage(const std::string& msg) {
  LOG(FATAL) << "Sending sync message to extension which doesn't support it!";
  return std::string();
}

}  // namespace extensions
}  // namespace xwalk
