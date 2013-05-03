// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/shell_content_renderer_client.h"

#include "base/callback.h"
#include "base/command_line.h"
#include "base/debug/debugger.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/render_view.h"
#include "content/public/test/layouttest_support.h"
#include "content/shell/shell_render_process_observer.h"
#include "content/shell/shell_switches.h"
#include "content/shell/webkit_test_runner.h"
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

namespace content {

namespace {
ShellContentRendererClient* g_renderer_client;
}

ShellContentRendererClient* ShellContentRendererClient::Get() {
  return g_renderer_client;
}

ShellContentRendererClient::ShellContentRendererClient() {
  DCHECK(!g_renderer_client);
  g_renderer_client = this;
  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree)) {
    EnableWebTestProxyCreation(
        base::Bind(&ShellContentRendererClient::WebTestProxyCreated,
                   base::Unretained(this)));
  }
}

ShellContentRendererClient::~ShellContentRendererClient() {
  g_renderer_client = NULL;
}

void ShellContentRendererClient::LoadHyphenDictionary(
    base::PlatformFile dict_file) {
  if (!hyphenator_)
    hyphenator_.reset(new webkit_glue::MockWebHyphenator);
  base::SeekPlatformFile(dict_file, base::PLATFORM_FILE_FROM_BEGIN, 0);
  hyphenator_->LoadDictionary(dict_file);
}

void ShellContentRendererClient::RenderThreadStarted() {
  shell_observer_.reset(new ShellRenderProcessObserver());
#if defined(OS_MACOSX)
  // We need to call this once before the sandbox was initialized to cache the
  // value.
  base::debug::BeingDebugged();
#endif
}

void ShellContentRendererClient::RenderViewCreated(RenderView* render_view) {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return;
  WebKitTestRunner* test_runner = WebKitTestRunner::Get(render_view);
  test_runner->Reset();
  render_view->GetWebView()->setSpellCheckClient(
      test_runner->proxy()->spellCheckClient());
  render_view->GetWebView()->setPermissionClient(
      ShellRenderProcessObserver::GetInstance()->test_interfaces()->testRunner()
          ->webPermissions());
  WebTestDelegate* delegate =
      ShellRenderProcessObserver::GetInstance()->test_delegate();
  if (delegate == static_cast<WebTestDelegate*>(test_runner))
    ShellRenderProcessObserver::GetInstance()->SetMainWindow(render_view);
}

bool ShellContentRendererClient::OverrideCreatePlugin(
    RenderView* render_view,
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
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return NULL;
#if defined(ENABLE_WEBRTC)
  WebTestInterfaces* interfaces =
      ShellRenderProcessObserver::GetInstance()->test_interfaces();
  return interfaces->createMediaStreamCenter(client);
#else
  return NULL;
#endif
}

WebRTCPeerConnectionHandler*
ShellContentRendererClient::OverrideCreateWebRTCPeerConnectionHandler(
    WebRTCPeerConnectionHandlerClient* client) {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return NULL;
#if defined(ENABLE_WEBRTC)
  WebTestInterfaces* interfaces =
      ShellRenderProcessObserver::GetInstance()->test_interfaces();
  return interfaces->createWebRTCPeerConnectionHandler(client);
#else
  return NULL;
#endif
}

WebClipboard* ShellContentRendererClient::OverrideWebClipboard() {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return NULL;
  if (!clipboard_)
    clipboard_.reset(new MockWebClipboardImpl);
  return clipboard_.get();
}

WebMimeRegistry* ShellContentRendererClient::OverrideWebMimeRegistry() {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return NULL;
  if (!mime_registry_)
    mime_registry_.reset(new TestShellWebMimeRegistryImpl);
  return mime_registry_.get();
}

WebHyphenator* ShellContentRendererClient::OverrideWebHyphenator() {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return NULL;
  if (!hyphenator_)
    hyphenator_.reset(new webkit_glue::MockWebHyphenator);
  return hyphenator_.get();

}

WebThemeEngine* ShellContentRendererClient::OverrideThemeEngine() {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return NULL;
  return ShellRenderProcessObserver::GetInstance()->test_interfaces()
      ->themeEngine();
}

void ShellContentRendererClient::WebTestProxyCreated(RenderView* render_view,
                                                     WebTestProxyBase* proxy) {
  WebKitTestRunner* test_runner = new WebKitTestRunner(render_view);
  test_runner->set_proxy(proxy);
  if (!ShellRenderProcessObserver::GetInstance()->test_delegate())
    ShellRenderProcessObserver::GetInstance()->SetTestDelegate(test_runner);
  proxy->setInterfaces(
      ShellRenderProcessObserver::GetInstance()->test_interfaces());
  test_runner->proxy()->setDelegate(
      ShellRenderProcessObserver::GetInstance()->test_delegate());
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

}  // namespace content
