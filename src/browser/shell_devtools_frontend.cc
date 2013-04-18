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

namespace content {

namespace {

// DevTools frontend path for inspector LayoutTests.
GURL GetDevToolsPathAsURL() {
  base::FilePath dir_exe;
  if (!PathService::Get(base::DIR_EXE, &dir_exe)) {
    NOTREACHED();
    return GURL();
  }
#if defined(OS_MACOSX)
  // On Mac, the executable is in
  // out/Release/Content Shell.app/Contents/MacOS/Content Shell.
  // We need to go up 3 directories to get to out/Release.
  dir_exe = dir_exe.AppendASCII("../../..");
#endif
  base::FilePath dev_tools_path = dir_exe.AppendASCII(
      "resources/inspector/devtools.html");
  return net::FilePathToFileURL(dev_tools_path);
}

}  // namespace

// static
ShellDevToolsFrontend* ShellDevToolsFrontend::Show(
    WebContents* inspected_contents) {
  Shell* shell = Shell::CreateNewWindow(inspected_contents->GetBrowserContext(),
                                        GURL(),
                                        NULL,
                                        MSG_ROUTING_NONE,
                                        gfx::Size());
  shell->set_is_devtools(true);
  ShellDevToolsFrontend* devtools_frontend = new ShellDevToolsFrontend(
      shell,
      DevToolsAgentHost::GetOrCreateFor(
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

ShellDevToolsFrontend::ShellDevToolsFrontend(Shell* frontend_shell,
                                             DevToolsAgentHost* agent_host)
    : WebContentsObserver(frontend_shell->web_contents()),
      inspected_shell_(NULL),
      frontend_shell_(frontend_shell),
      agent_host_(agent_host) {
  frontend_host_.reset(
      DevToolsClientHost::CreateDevToolsFrontendHost(web_contents(), this));
}

ShellDevToolsFrontend::~ShellDevToolsFrontend() {
  frontend_shell_->Close();
}

void ShellDevToolsFrontend::RenderViewCreated(
    RenderViewHost* render_view_host) {
  DevToolsClientHost::SetupDevToolsFrontendClient(
      web_contents()->GetRenderViewHost());
  DevToolsManager* manager = DevToolsManager::GetInstance();
  manager->RegisterDevToolsClientHostFor(agent_host_.get(),
                                         frontend_host_.get());
}

void ShellDevToolsFrontend::WebContentsDestroyed(WebContents* web_contents) {
  DevToolsManager::GetInstance()->ClientHostClosing(frontend_host_.get());
  inspected_shell_->CloseDevTools();
  delete this;
}

void ShellDevToolsFrontend::InspectedContentsClosing() {
  frontend_shell_->Close();
}

}  // namespace content
