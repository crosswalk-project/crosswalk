// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/renderer/shell_content_renderer_client.h"

#include "base/command_line.h"
#include "base/debug/debugger.h"
#include "cameo/src/common/shell_switches.h"
#include "content/public/common/content_switches.h"

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

void ShellContentRendererClient::RenderThreadStarted() {
#if defined(OS_MACOSX)
  // We need to call this once before the sandbox was initialized to cache the
  // value.
  base::debug::BeingDebugged();
#endif
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
