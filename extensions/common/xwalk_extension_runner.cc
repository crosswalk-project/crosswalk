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

void XWalkExtensionRunner::PostMessageToContext(scoped_ptr<base::Value> msg) {
  HandleMessageFromClient(msg.Pass());
}

void XWalkExtensionRunner::SendSyncMessageToContext(
    scoped_ptr<IPC::Message> ipc_reply, scoped_ptr<base::Value> msg) {
  return HandleSyncMessageFromClient(ipc_reply.Pass(), msg.Pass());
}

void XWalkExtensionRunner::PostMessageToClient(scoped_ptr<base::Value> msg) {
  client_->HandleMessageFromContext(this, msg.Pass());
}

void XWalkExtensionRunner::PostReplyMessageToClient(
    scoped_ptr<IPC::Message> ipc_reply, scoped_ptr<base::Value> msg) {
  client_->HandleReplyMessageFromContext(ipc_reply.Pass(), msg.Pass());
}

}  // namespace extensions
}  // namespace xwalk
