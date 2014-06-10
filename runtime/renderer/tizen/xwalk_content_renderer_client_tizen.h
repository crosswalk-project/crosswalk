// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_TIZEN_XWALK_CONTENT_RENDERER_CLIENT_TIZEN_H_
#define XWALK_RUNTIME_RENDERER_TIZEN_XWALK_CONTENT_RENDERER_CLIENT_TIZEN_H_

#include <string>

#include "xwalk/runtime/renderer/xwalk_content_renderer_client.h"

namespace xwalk {

class XWalkContentRendererClientTizen : public XWalkContentRendererClient {
 public:
  XWalkContentRendererClientTizen() : XWalkContentRendererClient() {}

  virtual bool WillSendRequest(blink::WebFrame* frame,
                               content::PageTransition transition_type,
                               const GURL& url,
                               const GURL& first_party_for_cookies,
                               GURL* new_url) OVERRIDE;

  virtual bool HasErrorPage(int http_status_code,
                            std::string* error_domain) OVERRIDE;

  virtual void GetNavigationErrorStrings(
      content::RenderView* render_view,
      blink::WebFrame* frame,
      const blink::WebURLRequest& failed_request,
      const blink::WebURLError& error,
      std::string* error_html,
      base::string16* error_description) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkContentRendererClientTizen);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_RENDERER_TIZEN_XWALK_CONTENT_RENDERER_CLIENT_TIZEN_H_
