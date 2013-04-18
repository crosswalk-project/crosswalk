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

namespace WebTestRunner {
class WebTestProxyBase;
}

namespace webkit_glue {
class MockWebHyphenator;
}

class MockWebClipboardImpl;
class TestShellWebMimeRegistryImpl;

namespace cameo {

class ShellRenderProcessObserver;

class ShellContentRendererClient : public content::ContentRendererClient {
 public:
  static ShellContentRendererClient* Get();

  ShellContentRendererClient();
  virtual ~ShellContentRendererClient();

  void LoadHyphenDictionary(base::PlatformFile dict_file);

  // ContentRendererClient implementation.
  virtual void RenderThreadStarted() OVERRIDE;
  virtual void RenderViewCreated(content::RenderView* render_view) OVERRIDE;
  virtual bool OverrideCreatePlugin(
      content::RenderView* render_view,
      WebKit::WebFrame* frame,
      const WebKit::WebPluginParams& params,
      WebKit::WebPlugin** plugin) OVERRIDE;
  virtual WebKit::WebMediaStreamCenter* OverrideCreateWebMediaStreamCenter(
      WebKit::WebMediaStreamCenterClient* client) OVERRIDE;
  virtual WebKit::WebRTCPeerConnectionHandler*
  OverrideCreateWebRTCPeerConnectionHandler(
      WebKit::WebRTCPeerConnectionHandlerClient* client) OVERRIDE;
  virtual WebKit::WebClipboard* OverrideWebClipboard() OVERRIDE;
  virtual WebKit::WebMimeRegistry* OverrideWebMimeRegistry() OVERRIDE;
  virtual WebKit::WebHyphenator* OverrideWebHyphenator() OVERRIDE;
  virtual WebKit::WebThemeEngine* OverrideThemeEngine() OVERRIDE;
  virtual bool AllowBrowserPlugin(
      WebKit::WebPluginContainer* container) const OVERRIDE;

 private:
  scoped_ptr<ShellRenderProcessObserver> shell_observer_;
};

}  // namespace cameo

#endif  // CAMEO_SRC_RENDERER_SHELL_CONTENT_RENDERER_CLIENT_H_