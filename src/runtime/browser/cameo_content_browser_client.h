// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RUNTIME_BROWSER_CAMEO_CONTENT_BROWSER_CLIENT_H_
#define CAMEO_SRC_RUNTIME_BROWSER_CAMEO_CONTENT_BROWSER_CLIENT_H_

#include "base/compiler_specific.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/common/main_function_params.h"

namespace content {
class BrowserContext;
class WebContents;
class WebContentsViewDelegate;
}

namespace net {
class URLRequestContextGetter;
}

namespace cameo {

class CameoBrowserMainParts;
class RuntimeContext;

class CameoContentBrowserClient : public content::ContentBrowserClient {
 public:
  static CameoContentBrowserClient* Get();

  CameoContentBrowserClient();
  virtual ~CameoContentBrowserClient();

  // ContentBrowserClient overrides.
  virtual content::BrowserMainParts* CreateBrowserMainParts(
      const content::MainFunctionParams& parameters) OVERRIDE;
  virtual net::URLRequestContextGetter* CreateRequestContext(
      content::BrowserContext* browser_context,
      content::ProtocolHandlerMap* protocol_handlers) OVERRIDE;
  virtual net::URLRequestContextGetter* CreateRequestContextForStoragePartition(
      content::BrowserContext* browser_context,
      const base::FilePath& partition_path,
      bool in_memory,
      content::ProtocolHandlerMap* protocol_handlers) OVERRIDE;
  virtual content::AccessTokenStore* CreateAccessTokenStore() OVERRIDE;
  virtual content::WebContentsViewDelegate* GetWebContentsViewDelegate(
      content::WebContents* web_contents) OVERRIDE;
  virtual void RenderProcessHostCreated(
      content::RenderProcessHost* host) OVERRIDE;

 private:
  net::URLRequestContextGetter* url_request_context_getter_;
  CameoBrowserMainParts* main_parts_;

  DISALLOW_COPY_AND_ASSIGN(CameoContentBrowserClient);
};

}  // namespace cameo

#endif  // CAMEO_SRC_RUNTIME_BROWSER_CAMEO_CONTENT_BROWSER_CLIENT_H_
