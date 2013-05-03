// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/shell_render_process_observer.h"

#include "base/command_line.h"
#include "content/public/common/content_client.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/test/layouttest_support.h"
#include "content/shell/shell_content_renderer_client.h"
#include "content/shell/shell_messages.h"
#include "content/shell/shell_switches.h"
#include "content/shell/webkit_test_runner.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"
#include "third_party/WebKit/Tools/DumpRenderTree/chromium/TestRunner/public/WebTestInterfaces.h"
#include "webkit/glue/webkit_glue.h"
#include "webkit/support/gc_extension.h"

using WebKit::WebFrame;
using WebTestRunner::WebTestDelegate;
using WebTestRunner::WebTestInterfaces;

namespace content {

namespace {
ShellRenderProcessObserver* g_instance = NULL;
}

// static
ShellRenderProcessObserver* ShellRenderProcessObserver::GetInstance() {
  return g_instance;
}

ShellRenderProcessObserver::ShellRenderProcessObserver()
    : main_test_runner_(NULL),
      test_delegate_(NULL) {
  CHECK(!g_instance);
  g_instance = this;
  RenderThread::Get()->AddObserver(this);
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return;
  DisableAppCacheLogging();
  EnableDevToolsFrontendTesting();
  EnableShortCircuitSizeUpdates();
  DoNotSendFocusEvents();
  DisableNavigationErrorPages();
}

ShellRenderProcessObserver::~ShellRenderProcessObserver() {
  CHECK(g_instance == this);
  g_instance = NULL;
}

void ShellRenderProcessObserver::SetTestDelegate(WebTestDelegate* delegate) {
  test_interfaces_->setDelegate(delegate);
  test_delegate_ = delegate;
}

void ShellRenderProcessObserver::SetMainWindow(RenderView* view) {
  WebKitTestRunner* test_runner = WebKitTestRunner::Get(view);
  test_interfaces_->setWebView(view->GetWebView(), test_runner->proxy());
  main_test_runner_ = test_runner;
}

void ShellRenderProcessObserver::WebKitInitialized() {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return;

  // We always expose GC to layout tests.
  webkit_glue::SetJavaScriptFlags(" --expose-gc");
  RenderThread::Get()->RegisterExtension(extensions_v8::GCExtension::Get());

  test_interfaces_.reset(new WebTestInterfaces);
  test_interfaces_->resetAll();
}

bool ShellRenderProcessObserver::OnControlMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ShellRenderProcessObserver, message)
    IPC_MESSAGE_HANDLER(ShellViewMsg_SetWebKitSourceDir, OnSetWebKitSourceDir)
    IPC_MESSAGE_HANDLER(ShellViewMsg_LoadHyphenDictionary,
                        OnLoadHyphenDictionary)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void ShellRenderProcessObserver::OnSetWebKitSourceDir(
    const base::FilePath& webkit_source_dir) {
  webkit_source_dir_ = webkit_source_dir;
}

void ShellRenderProcessObserver::OnLoadHyphenDictionary(
    const IPC::PlatformFileForTransit& dict_file) {
  ShellContentRendererClient::Get()->LoadHyphenDictionary(
      IPC::PlatformFileForTransitToPlatformFile(dict_file));
}

}  // namespace content
