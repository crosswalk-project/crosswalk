// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/runtime/browser/cameo_content_browser_client.h"

#include "cameo/src/extensions/browser/cameo_extension_host.h"
#include "cameo/src/runtime/browser/cameo_browser_main_parts.h"
#include "cameo/src/runtime/browser/geolocation/cameo_access_token_store.h"
#include "cameo/src/runtime/browser/media/media_capture_devices_dispatcher.h"
#include "cameo/src/runtime/browser/runtime_context.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "content/public/common/main_function_params.h"
#include "net/url_request/url_request_context_getter.h"

namespace cameo {

namespace {

// The application-wide singleton of ContentBrowserClient impl.
CameoContentBrowserClient* g_browser_client = NULL;

}  // namespace

// static
CameoContentBrowserClient* CameoContentBrowserClient::Get() {
  return g_browser_client;
}

CameoContentBrowserClient::CameoContentBrowserClient() {
  DCHECK(!g_browser_client);
  g_browser_client = this;
}

CameoContentBrowserClient::~CameoContentBrowserClient() {
  DCHECK(g_browser_client);
  g_browser_client = NULL;
}

content::BrowserMainParts* CameoContentBrowserClient::CreateBrowserMainParts(
    const content::MainFunctionParams& parameters) {
  return new CameoBrowserMainParts(parameters);
}

net::URLRequestContextGetter* CameoContentBrowserClient::CreateRequestContext(
    content::BrowserContext* browser_context,
    content::ProtocolHandlerMap* protocol_handlers) {
  url_request_context_getter_ = static_cast<RuntimeContext*>(browser_context)->
      CreateRequestContext(protocol_handlers);
  return url_request_context_getter_;
}

net::URLRequestContextGetter*
CameoContentBrowserClient::CreateRequestContextForStoragePartition(
    content::BrowserContext* browser_context,
    const base::FilePath& partition_path,
    bool in_memory,
    content::ProtocolHandlerMap* protocol_handlers) {
  return static_cast<RuntimeContext*>(browser_context)->
      CreateRequestContextForStoragePartition(
          partition_path, in_memory, protocol_handlers);
}

content::AccessTokenStore* CameoContentBrowserClient::CreateAccessTokenStore() {
  return new CameoAccessTokenStore(url_request_context_getter_);
}

content::WebContentsViewDelegate*
CameoContentBrowserClient::GetWebContentsViewDelegate(
    content::WebContents* web_contents) {
  return NULL;
}

void CameoContentBrowserClient::RenderProcessHostCreated(
    content::RenderProcessHost* host) {

  extensions::CameoExtensionHost* extension_host =
      new extensions::CameoExtensionHost;

  // Register the extensions here.

  // FIXME(cmarcelo): CameoExtensionHost shouldn't be a MessageFilter,
  // we want a clearer lifetime, tied to the browser process or the
  // render process host.
  host->GetChannel()->AddFilter(extension_host);
}

content::MediaObserver* CameoContentBrowserClient::GetMediaObserver() {
  return CameoMediaCaptureDevicesDispatcher::GetInstance();
}

}  // namespace cameo
