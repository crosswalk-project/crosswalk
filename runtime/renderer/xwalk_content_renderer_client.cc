// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/runtime/renderer/xwalk_content_renderer_client.h"

#include "cameo/extensions/renderer/xwalk_extension_renderer_controller.h"

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

void XWalkContentRendererClient::RenderViewCreated(
    content::RenderView* render_view) {
  extension_controller_->RenderViewCreated(render_view);
}

}  // namespace xwalk
