// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/browser/shell_devtools_frontend.h"

#include "base/command_line.h"
#include "base/path_service.h"
#include "cameo/src/common/shell_switches.h"
#include "cameo/src/browser/shell.h"
#include "cameo/src/browser/shell_browser_context.h"
#include "cameo/src/browser/shell_browser_main_parts.h"
#include "cameo/src/browser/shell_content_browser_client.h"
#include "cameo/src/browser/shell_devtools_delegate.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/devtools_manager.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/content_client.h"

#include "net/base/net_util.h"

namespace cameo {

// static
ShellDevToolsFrontend* ShellDevToolsFrontend::Show(
    content::WebContents* inspected_contents) {
  Shell* shell = Shell::CreateNewWindow(inspected_contents->GetBrowserContext(),
                                        GURL(),
                                        NULL,
                                        MSG_ROUTING_NONE,
                                        gfx::Size());
  shell->set_is_devtools(true);
  ShellDevToolsFrontend* devtools_frontend = new ShellDevToolsFrontend(
      shell,
      content::DevToolsAgentHost::GetOrCreateFor(
          inspected_contents->GetRenderViewHost()));

  ShellDevToolsDelegate* delegate = ShellContentBrowserClient::Get()->
      shell_browser_main_parts()->devtools_delegate();

  shell->LoadURL(delegate->devtools_http_handler()->GetFrontendURL(NULL));

  return devtools_frontend;
}

void ShellDevToolsFrontend::Focus() {
  web_contents()->GetView()->Focus();
}

void ShellDevToolsFrontend::Close() {
  frontend_shell_->Close();
}

ShellDevToolsFrontend::ShellDevToolsFrontend(
    Shell* frontend_shell, content::DevToolsAgentHost* agent_host)
    : WebContentsObserver(frontend_shell->web_contents()),
      inspected_shell_(NULL),
      frontend_shell_(frontend_shell),
      agent_host_(agent_host) {
  frontend_host_.reset(
      content::DevToolsClientHost::CreateDevToolsFrontendHost(
          web_contents(), this));
}

ShellDevToolsFrontend::~ShellDevToolsFrontend() {
  frontend_shell_->Close();
}

void ShellDevToolsFrontend::RenderViewCreated(
    content::RenderViewHost* render_view_host) {
  content::DevToolsClientHost::SetupDevToolsFrontendClient(
      web_contents()->GetRenderViewHost());
  content::DevToolsManager* manager = content::DevToolsManager::GetInstance();
  manager->RegisterDevToolsClientHostFor(agent_host_.get(),
                                         frontend_host_.get());
}

void ShellDevToolsFrontend::WebContentsDestroyed(
    content::WebContents* web_contents) {
  content::DevToolsManager::GetInstance()->ClientHostClosing(
      frontend_host_.get());
  inspected_shell_->CloseDevTools();
  delete this;
}

void ShellDevToolsFrontend::InspectedContentsClosing() {
  frontend_shell_->Close();
}

}  // namespace cameo
