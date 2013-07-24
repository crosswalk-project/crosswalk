// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension_runner.h"

namespace xwalk {
namespace extensions {

XWalkExtensionRunner::XWalkExtensionRunner(const std::string& extension_name,
                                           Client* client)
    : client_(client),
      extension_name_(extension_name) {}

XWalkExtensionRunner::~XWalkExtensionRunner() {}

void XWalkExtensionRunner::PostMessageToContext(const std::string& msg) {
  HandleMessageFromClient(msg);
}

void XWalkExtensionRunner::PostMessageToClient(const std::string& msg) {
  client_->HandleMessageFromContext(this, msg);
}


}  // namespace extensions
}  // namespace xwalk
