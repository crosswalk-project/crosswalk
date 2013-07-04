// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/runtime/renderer/cameo_content_renderer_client.h"

#include "cameo/extensions/renderer/xwalk_extension_renderer_controller.h"

namespace cameo {

namespace {
CameoContentRendererClient* g_renderer_client;
}

CameoContentRendererClient* CameoContentRendererClient::Get() {
  return g_renderer_client;
}

CameoContentRendererClient::CameoContentRendererClient() {
  DCHECK(!g_renderer_client);
  g_renderer_client = this;
}

CameoContentRendererClient::~CameoContentRendererClient() {
  g_renderer_client = NULL;
}

void CameoContentRendererClient::RenderThreadStarted() {
  extension_controller_.reset(new extensions::XWalkExtensionRendererController);
}

void CameoContentRendererClient::RenderViewCreated(
    content::RenderView* render_view) {
  extension_controller_->RenderViewCreated(render_view);
}

}  // namespace cameo
