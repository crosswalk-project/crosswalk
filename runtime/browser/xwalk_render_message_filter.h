// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_RUNTIME_BROWSER_XWALK_RENDER_MESSAGE_FILTER_H_
#define XWALK_RUNTIME_BROWSER_XWALK_RENDER_MESSAGE_FILTER_H_

#include "content/public/browser/browser_message_filter.h"
#include "url/gurl.h"

namespace xwalk {
// XWalkBrowserMessageFilter response to recieve and send message between
// browser process and renderer process.
class XWalkRenderMessageFilter : public content::BrowserMessageFilter {
 public:
  XWalkRenderMessageFilter();
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
#if defined(OS_TIZEN)
  void OnOpenLinkExternal(const GURL& url);
#endif
  virtual ~XWalkRenderMessageFilter() {}

  DISALLOW_COPY_AND_ASSIGN(XWalkRenderMessageFilter);
};
}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_RENDER_MESSAGE_FILTER_H_
