// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_RUNTIME_RENDERER_XWALK_CONTENT_RENDERER_CLIENT_H_
#define CAMEO_RUNTIME_RENDERER_XWALK_CONTENT_RENDERER_CLIENT_H_

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/platform_file.h"
#include "content/public/renderer/content_renderer_client.h"

namespace cameo {

namespace extensions {
class XWalkExtensionRendererController;
}

class XWalkContentRendererClient : public content::ContentRendererClient {
 public:
  static XWalkContentRendererClient* Get();

  XWalkContentRendererClient();
  virtual ~XWalkContentRendererClient();

  // ContentRendererClient implementation.
  virtual void RenderThreadStarted() OVERRIDE;
  virtual void RenderViewCreated(content::RenderView* render_view) OVERRIDE;

 private:
  scoped_ptr<extensions::XWalkExtensionRendererController>
      extension_controller_;

  DISALLOW_COPY_AND_ASSIGN(XWalkContentRendererClient);
};

}  // namespace cameo

#endif  // CAMEO_RUNTIME_RENDERER_XWALK_CONTENT_RENDERER_CLIENT_H_
