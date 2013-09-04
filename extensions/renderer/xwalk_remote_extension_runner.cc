// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_remote_extension_runner.h"

#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/renderer/xwalk_extension_client.h"

namespace xwalk {
namespace extensions {

XWalkRemoteExtensionRunner::XWalkRemoteExtensionRunner(Client* client,
    XWalkExtensionClient* extension_client, int64_t instance_id)
    : client_(client),
      instance_id_(instance_id),
      extension_client_(extension_client) {}

XWalkRemoteExtensionRunner::~XWalkRemoteExtensionRunner() {}

void XWalkRemoteExtensionRunner::PostMessageToNative(
    scoped_ptr<base::Value> msg) {
  extension_client_->PostMessageToNative(instance_id_, msg.Pass());
}

scoped_ptr<base::Value> XWalkRemoteExtensionRunner::SendSyncMessageToNative(
    scoped_ptr<base::Value> msg) {
  scoped_ptr<base::Value> reply(extension_client_->SendSyncMessageToNative(
      instance_id_, msg.Pass()));
  return reply.Pass();
}

void XWalkRemoteExtensionRunner::PostMessageToJS(
    const base::Value& msg) {
  client_->HandleMessageFromNative(msg);
}

void XWalkRemoteExtensionRunner::Destroy() {
  extension_client_->DestroyInstance(instance_id_);
}

}  // namespace extensions
}  // namespace xwalk

