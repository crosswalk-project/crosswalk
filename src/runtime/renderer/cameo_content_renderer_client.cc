// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/runtime/renderer/cameo_content_renderer_client.h"

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
}

}  // namespace cameo
