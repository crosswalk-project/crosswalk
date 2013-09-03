// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_XWALK_CONTENT_RENDERER_CLIENT_H_
#define XWALK_RUNTIME_RENDERER_XWALK_CONTENT_RENDERER_CLIENT_H_

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/platform_file.h"
#include "content/public/renderer/content_renderer_client.h"

namespace xwalk {

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
  virtual void DidCreateScriptContext(
      WebKit::WebFrame* frame, v8::Handle<v8::Context> context,
      int extension_group, int world_id) OVERRIDE;
  virtual void WillReleaseScriptContext(WebKit::WebFrame* frame,
                                        v8::Handle<v8::Context>,
                                        int world_id) OVERRIDE;

 private:
  scoped_ptr<extensions::XWalkExtensionRendererController>
      extension_controller_;

  DISALLOW_COPY_AND_ASSIGN(XWalkContentRendererClient);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_RENDERER_XWALK_CONTENT_RENDERER_CLIENT_H_
