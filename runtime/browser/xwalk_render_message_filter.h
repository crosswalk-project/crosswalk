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
#if defined(OS_ANDROID)
  explicit XWalkRenderMessageFilter(int process_id);
  // BrowserMessageFilter methods.
  void OverrideThreadForMessage(const IPC::Message& message,
                                content::BrowserThread::ID* thread) override;
#endif
  bool OnMessageReceived(const IPC::Message& message) override;

 private:
  void OnOpenLinkExternal(const GURL& url);
#if defined(OS_ANDROID)
  void OnSubFrameCreated(int parent_render_frame_id, int child_render_frame_id);
  void OnShouldOverrideUrlLoading(int routing_id,
                                  const base::string16& url,
                                  bool has_user_gesture,
                                  bool is_redirect,
                                  bool is_main_frame,
                                  bool* ignore_navigation);
#endif
  ~XWalkRenderMessageFilter() override {}

#if defined(OS_ANDROID)
  int process_id_;
#endif

  DISALLOW_COPY_AND_ASSIGN(XWalkRenderMessageFilter);
};
}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_RENDER_MESSAGE_FILTER_H_
