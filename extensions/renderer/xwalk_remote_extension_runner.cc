// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_remote_extension_runner.h"

#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/renderer/xwalk_extension_client.h"
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

namespace {

// Regular base::Value doesn't have param traits, so can't be passed as is
// through IPC. We wrap it in a base::ListValue that have traits before
// exchanging.
//
// Implementing param traits for base::Value is not a viable option at the
// moment (would require fork base::Value and create a new empty type).
scoped_ptr<base::ListValue> WrapValueInList(scoped_ptr<base::Value> value) {
  if (!value)
    return scoped_ptr<base::ListValue>();
  scoped_ptr<base::ListValue> list_value(new base::ListValue);
  list_value->Append(value.release());
  return list_value.Pass();
}

}  // namespace

void XWalkRemoteExtensionRunner::PostMessageToNative(
    scoped_ptr<base::Value> msg) {
  // FIXME(jeez): Remove this.
  handler_->PostMessageToExtension(frame_id_, extension_name_, msg.Pass());

  // Uncomenting this causes a crash since msg is owned by the IPC channel.
  // scoped_ptr<base::ListValue> wrapped_msg = WrapValueInList(msg.Pass());
  // extension_client_->Send(new XWalkExtensionServerMsg_PostMessageToNative(
  //     instance_id_, *wrapped_msg));
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

