// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/runtime/browser/devtools/cameo_devtools_delegate.h"

#include <string>

#include "base/bind.h"
#include "cameo/runtime/browser/runtime.h"
#include "cameo/runtime/browser/runtime_context.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/url_constants.h"
#include "grit/cameo_resources.h"
#include "net/socket/tcp_listen_socket.h"
#include "ui/base/resource/resource_bundle.h"

namespace cameo {

CameoDevToolsDelegate::CameoDevToolsDelegate(RuntimeContext* runtime_context)
    : runtime_context_(runtime_context) {
}

CameoDevToolsDelegate::~CameoDevToolsDelegate() {
}

std::string CameoDevToolsDelegate::GetDiscoveryPageHTML() {
  return ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_DEVTOOLS_FRONTEND_PAGE_HTML).as_string();
}

bool CameoDevToolsDelegate::BundlesFrontendResources() {
  return true;
}

base::FilePath CameoDevToolsDelegate::GetDebugFrontendDir() {
  return base::FilePath();
}

std::string CameoDevToolsDelegate::GetPageThumbnailData(const GURL& url) {
  return std::string();
}

content::RenderViewHost* CameoDevToolsDelegate::CreateNewTarget() {
  Runtime* runtime = Runtime::Create(runtime_context_,
                                     GURL(chrome::kAboutBlankURL));
  return runtime->web_contents()->GetRenderViewHost();
}

content::DevToolsHttpHandlerDelegate::TargetType
CameoDevToolsDelegate::GetTargetType(content::RenderViewHost*) {
  return kTargetTypeOther;
}

std::string CameoDevToolsDelegate::GetViewDescription(
    content::RenderViewHost*) {
  return std::string();
}

scoped_refptr<net::StreamListenSocket>
CameoDevToolsDelegate::CreateSocketForTethering(
      net::StreamListenSocket::Delegate* delegate,
      std::string* name) {
  return NULL;
}

}  // namespace cameo
