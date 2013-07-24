// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_CONTENT_BROWSER_CLIENT_H_
#define XWALK_RUNTIME_BROWSER_XWALK_CONTENT_BROWSER_CLIENT_H_

#include <vector>

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

namespace xwalk {

class XWalkBrowserMainParts;
class RuntimeContext;

class XWalkContentBrowserClient : public content::ContentBrowserClient {
 public:
  static XWalkContentBrowserClient* Get();
#if defined(OS_ANDROID)
  static RuntimeContext* GetRuntimeContext();
#endif

  XWalkContentBrowserClient();
  virtual ~XWalkContentBrowserClient();

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
  virtual content::MediaObserver* GetMediaObserver() OVERRIDE;

#if defined(OS_ANDROID)
  virtual void GetAdditionalMappedFilesForChildProcess(
      const CommandLine& command_line,
      int child_process_id,
      std::vector<content::FileDescriptorInfo>* mappings) OVERRIDE;
#endif

 private:
  net::URLRequestContextGetter* url_request_context_getter_;
  XWalkBrowserMainParts* main_parts_;

  DISALLOW_COPY_AND_ASSIGN(XWalkContentBrowserClient);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_CONTENT_BROWSER_CLIENT_H_
