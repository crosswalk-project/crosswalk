// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/browser/shell_devtools_delegate.h"

#include "base/bind.h"
#include "cameo/src/browser/shell.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/url_constants.h"
#include "grit/cameo_resources.h"
#include "net/socket/tcp_listen_socket.h"
#include "ui/base/resource/resource_bundle.h"

#if defined(OS_ANDROID)
#include "content/public/browser/android/devtools_auth.h"
#include "net/socket/unix_domain_socket_posix.h"

namespace {
const char kSocketName[] = "content_shell_devtools_remote";
}
#endif

namespace cameo {

ShellDevToolsDelegate::ShellDevToolsDelegate(
    content::BrowserContext* browser_context, int port)
    : browser_context_(browser_context) {
  devtools_http_handler_ = content::DevToolsHttpHandler::Start(
#if defined(OS_ANDROID)
      new net::UnixDomainSocketWithAbstractNamespaceFactory(
          kSocketName, base::Bind(&CanUserConnectToDevTools)),
#else
      new net::TCPListenSocketFactory("127.0.0.1", port),
#endif
      std::string(),
      this);
}

ShellDevToolsDelegate::~ShellDevToolsDelegate() {
}

void ShellDevToolsDelegate::Stop() {
  // The call below destroys this.
  devtools_http_handler_->Stop();
}

std::string ShellDevToolsDelegate::GetDiscoveryPageHTML() {
  return ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_CONTENT_SHELL_DEVTOOLS_DISCOVERY_PAGE).as_string();
}

bool ShellDevToolsDelegate::BundlesFrontendResources() {
  return true;
}

base::FilePath ShellDevToolsDelegate::GetDebugFrontendDir() {
  return base::FilePath();
}

std::string ShellDevToolsDelegate::GetPageThumbnailData(const GURL& url) {
  return std::string();
}

content::RenderViewHost* ShellDevToolsDelegate::CreateNewTarget() {
  Shell* shell = Shell::CreateNewWindow(browser_context_,
                                        GURL(chrome::kAboutBlankURL),
                                        NULL,
                                        MSG_ROUTING_NONE,
                                        gfx::Size());
  return shell->web_contents()->GetRenderViewHost();
}

content::DevToolsHttpHandlerDelegate::TargetType
ShellDevToolsDelegate::GetTargetType(content::RenderViewHost*) {
  return kTargetTypeTab;
}

std::string ShellDevToolsDelegate::GetViewDescription(
    content::RenderViewHost*) {
  return std::string();
}

}  // namespace cameo