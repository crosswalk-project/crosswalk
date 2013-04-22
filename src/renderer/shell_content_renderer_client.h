// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RENDERER_SHELL_CONTENT_RENDERER_CLIENT_H_
#define CAMEO_SRC_RENDERER_SHELL_CONTENT_RENDERER_CLIENT_H_

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/platform_file.h"
#include "content/public/renderer/content_renderer_client.h"

namespace WebKit {
class WebFrame;
class WebPlugin;
struct WebPluginParams;
}

namespace cameo {

class ShellRenderProcessObserver;

class ShellContentRendererClient : public content::ContentRendererClient {
 public:
  static ShellContentRendererClient* Get();

  ShellContentRendererClient();
  virtual ~ShellContentRendererClient();

  // ContentRendererClient implementation.
  virtual void RenderThreadStarted() OVERRIDE;
  virtual bool AllowBrowserPlugin(
      WebKit::WebPluginContainer* container) const OVERRIDE;
};

}  // namespace cameo

#endif  // CAMEO_SRC_RENDERER_SHELL_CONTENT_RENDERER_CLIENT_H_
