// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/devtools/xwalk_devtools_delegate.h"

#include <string>

#include "base/bind.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/url_constants.h"
#include "grit/xwalk_resources.h"
#include "net/socket/tcp_listen_socket.h"
#include "ui/base/resource/resource_bundle.h"

namespace xwalk {

XWalkDevToolsDelegate::XWalkDevToolsDelegate(RuntimeContext* runtime_context)
    : runtime_context_(runtime_context) {
}

XWalkDevToolsDelegate::~XWalkDevToolsDelegate() {
}

std::string XWalkDevToolsDelegate::GetDiscoveryPageHTML() {
  return ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_DEVTOOLS_FRONTEND_PAGE_HTML).as_string();
}

bool XWalkDevToolsDelegate::BundlesFrontendResources() {
  return true;
}

base::FilePath XWalkDevToolsDelegate::GetDebugFrontendDir() {
  return base::FilePath();
}

std::string XWalkDevToolsDelegate::GetPageThumbnailData(const GURL& url) {
  return std::string();
}

content::RenderViewHost* XWalkDevToolsDelegate::CreateNewTarget() {
  Runtime* runtime = Runtime::CreateWithDefaultWindow(
      runtime_context_, GURL(content::kAboutBlankURL));
  return runtime->web_contents()->GetRenderViewHost();
}

content::DevToolsHttpHandlerDelegate::TargetType
XWalkDevToolsDelegate::GetTargetType(content::RenderViewHost*) {
  return kTargetTypeOther;
}

std::string XWalkDevToolsDelegate::GetViewDescription(
    content::RenderViewHost*) {
  return std::string();
}

scoped_refptr<net::StreamListenSocket>
XWalkDevToolsDelegate::CreateSocketForTethering(
      net::StreamListenSocket::Delegate* delegate,
      std::string* name) {
  return NULL;
}

}  // namespace xwalk
