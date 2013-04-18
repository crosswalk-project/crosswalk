// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/renderer/shell_content_renderer_client.h"

#include "base/callback.h"
#include "base/command_line.h"
#include "base/debug/debugger.h"
#include "cameo/src/renderer/shell_render_process_observer.h"
#include "cameo/src/common/shell_switches.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/render_view.h"
#include "content/public/test/layouttest_support.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebMediaStreamCenter.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebPluginParams.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"
#include "third_party/WebKit/Tools/DumpRenderTree/chromium/TestRunner/public/WebTestInterfaces.h"
#include "third_party/WebKit/Tools/DumpRenderTree/chromium/TestRunner/public/WebTestProxy.h"
#include "third_party/WebKit/Tools/DumpRenderTree/chromium/TestRunner/public/WebTestRunner.h"
#include "v8/include/v8.h"
#include "webkit/mocks/mock_webhyphenator.h"
#include "webkit/tools/test_shell/mock_webclipboard_impl.h"
#include "webkit/tools/test_shell/test_shell_webmimeregistry_impl.h"

using WebKit::WebClipboard;
using WebKit::WebFrame;
using WebKit::WebHyphenator;
using WebKit::WebMediaStreamCenter;
using WebKit::WebMediaStreamCenterClient;
using WebKit::WebMimeRegistry;
using WebKit::WebPlugin;
using WebKit::WebPluginParams;
using WebKit::WebRTCPeerConnectionHandler;
using WebKit::WebRTCPeerConnectionHandlerClient;
using WebKit::WebThemeEngine;
using WebTestRunner::WebTestDelegate;
using WebTestRunner::WebTestInterfaces;
using WebTestRunner::WebTestProxyBase;

namespace cameo {

namespace {
ShellContentRendererClient* g_renderer_client;
}

ShellContentRendererClient* ShellContentRendererClient::Get() {
  return g_renderer_client;
}

ShellContentRendererClient::ShellContentRendererClient() {
  DCHECK(!g_renderer_client);
  g_renderer_client = this;
}

ShellContentRendererClient::~ShellContentRendererClient() {
  g_renderer_client = NULL;
}

void ShellContentRendererClient::LoadHyphenDictionary(
    base::PlatformFile dict_file) {
}

void ShellContentRendererClient::RenderThreadStarted() {
  shell_observer_.reset(new ShellRenderProcessObserver());
#if defined(OS_MACOSX)
  // We need to call this once before the sandbox was initialized to cache the
  // value.
  base::debug::BeingDebugged();
#endif
}

void ShellContentRendererClient::RenderViewCreated(
    content::RenderView* render_view) {
  //TODO(nhu): implementation.
}

bool ShellContentRendererClient::OverrideCreatePlugin(
    content::RenderView* render_view,
    WebFrame* frame,
    const WebPluginParams& params,
    WebPlugin** plugin) {
  std::string mime_type = params.mimeType.utf8();
  if (mime_type == content::kBrowserPluginMimeType) {
    // Allow browser plugin in content_shell only if it is forced by flag.
    // Returning true here disables the plugin.
    return !CommandLine::ForCurrentProcess()->HasSwitch(
        switches::kEnableBrowserPluginForAllViewTypes);
  }
  return false;
}

WebMediaStreamCenter*
ShellContentRendererClient::OverrideCreateWebMediaStreamCenter(
    WebMediaStreamCenterClient* client) {
  //TODO(nhu): implementation.
  return NULL;
}

WebRTCPeerConnectionHandler*
ShellContentRendererClient::OverrideCreateWebRTCPeerConnectionHandler(
    WebRTCPeerConnectionHandlerClient* client) {
  //TODO(nhu): implementation.
  return NULL;
}

WebClipboard* ShellContentRendererClient::OverrideWebClipboard() {
  return NULL;
}

WebMimeRegistry* ShellContentRendererClient::OverrideWebMimeRegistry() {
  return NULL;
}

WebHyphenator* ShellContentRendererClient::OverrideWebHyphenator() {
  return NULL;
}

WebThemeEngine* ShellContentRendererClient::OverrideThemeEngine() {
  return NULL;
}

bool ShellContentRendererClient::AllowBrowserPlugin(
    WebKit::WebPluginContainer* container) const {
  if (CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableBrowserPluginForAllViewTypes)) {
    // Allow BrowserPlugin if forced by command line flag. This is generally
    // true for tests.
    return true;
  }
  return ContentRendererClient::AllowBrowserPlugin(container);
}

}  // namespace cameo