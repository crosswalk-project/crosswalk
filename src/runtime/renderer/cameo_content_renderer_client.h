// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RUNTIME_RENDERER_CAMEO_CONTENT_RENDERER_CLIENT_H_
#define CAMEO_SRC_RUNTIME_RENDERER_CAMEO_CONTENT_RENDERER_CLIENT_H_

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/platform_file.h"
#include "content/public/renderer/content_renderer_client.h"

namespace cameo {

class CameoContentRendererClient : public content::ContentRendererClient {
 public:
  static CameoContentRendererClient* Get();

  CameoContentRendererClient();
  virtual ~CameoContentRendererClient();

  // ContentRendererClient implementation.
  virtual void RenderThreadStarted() OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(CameoContentRendererClient);
};

}  // namespace cameo

#endif  // CAMEO_SRC_RUNTIME_RENDERER_CAMEO_CONTENT_RENDERER_CLIENT_H_
