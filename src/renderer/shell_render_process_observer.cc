// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/renderer/shell_render_process_observer.h"

#include "base/command_line.h"
#include "cameo/src/renderer/shell_content_renderer_client.h"
#include "cameo/src/common/shell_messages.h"
#include "cameo/src/common/shell_switches.h"
#include "content/public/common/content_client.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/test/layouttest_support.h"
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

ShellRenderProcessObserver::ShellRenderProcessObserver() {
  CHECK(!g_instance);
  g_instance = this;
  RenderThread::Get()->AddObserver(this);
}

ShellRenderProcessObserver::~ShellRenderProcessObserver() {
  CHECK(g_instance == this);
  g_instance = NULL;
}

void ShellRenderProcessObserver::WebKitInitialized() {
}

bool ShellRenderProcessObserver::OnControlMessageReceived(
    const IPC::Message& message) {
  bool handled = false;

  return handled;
}

}  // namespace content
