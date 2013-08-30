// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_remote_extension_runner.h"

#include "xwalk/extensions/renderer/xwalk_extension_render_view_handler.h"

namespace xwalk {
namespace extensions {

XWalkRemoteExtensionRunner::XWalkRemoteExtensionRunner(
    XWalkExtensionRenderViewHandler* handler, int64_t frame_id,
    const std::string& extension_name, Client* client,
    XWalkExtensionClient* extension_client, int64_t instance_id)
    : client_(client),
      extension_name_(extension_name),
      handler_(handler),
      frame_id_(frame_id),
      instance_id_(instance_id),
      extension_client_(extension_client) {}

XWalkRemoteExtensionRunner::~XWalkRemoteExtensionRunner() {}

void XWalkRemoteExtensionRunner::PostMessageToNative(
    scoped_ptr<base::Value> msg) {
  // FIXME(jeez): Remove this.
  handler_->PostMessageToExtension(frame_id_, extension_name_, msg.Pass());
}

scoped_ptr<base::Value> XWalkRemoteExtensionRunner::SendSyncMessageToNative(
    scoped_ptr<base::Value> msg) {
  scoped_ptr<base::Value> reply(handler_->SendSyncMessageToExtension(
      frame_id_, extension_name_, msg.Pass()));
  return reply.Pass();
}

void XWalkRemoteExtensionRunner::PostMessageToJS(
    const base::Value& msg) {
  client_->HandleMessageFromNative(msg);
}

}  // namespace extensions
}  // namespace xwalk

