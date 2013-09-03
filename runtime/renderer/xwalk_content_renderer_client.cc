// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/xwalk_content_renderer_client.h"

#include "xwalk/extensions/renderer/xwalk_extension_renderer_controller.h"

namespace xwalk {

namespace {
XWalkContentRendererClient* g_renderer_client;
}

XWalkContentRendererClient* XWalkContentRendererClient::Get() {
  return g_renderer_client;
}

XWalkContentRendererClient::XWalkContentRendererClient() {
  DCHECK(!g_renderer_client);
  g_renderer_client = this;
}

XWalkContentRendererClient::~XWalkContentRendererClient() {
  g_renderer_client = NULL;
}

void XWalkContentRendererClient::RenderThreadStarted() {
  extension_controller_.reset(new extensions::XWalkExtensionRendererController);
}

void XWalkContentRendererClient::DidCreateScriptContext(
    WebKit::WebFrame* frame, v8::Handle<v8::Context> context,
    int extension_group, int world_id) {
  extension_controller_->DidCreateScriptContext(frame, context);
}

void XWalkContentRendererClient::WillReleaseScriptContext(
    WebKit::WebFrame* frame, v8::Handle<v8::Context> context, int world_id) {
  extension_controller_->WillReleaseScriptContext(frame, context);
}

}  // namespace xwalk
