// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_RUNTIME_BROWSER_XWALK_RENDER_MESSAGE_FILTER_H_
#define XWALK_RUNTIME_BROWSER_XWALK_RENDER_MESSAGE_FILTER_H_

#include <string>
#include <vector>

#include "base/strings/string16.h"
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
#endif
  bool OnMessageReceived(const IPC::Message& message) override;

 private:
  void OnOpenLinkExternal(const GURL& url);
#if defined(ENABLE_PEPPER_CDMS)
  // Returns whether any internal plugin supporting |mime_type| is registered
  // and enabled. Does not determine whether the plugin can actually be
  // instantiated (e.g. whether it has all its dependencies).
  // When the returned *|is_available| is true, |additional_param_names| and
  // |additional_param_values| contain the name-value pairs, if any, specified
  // for the *first* non-disabled plugin found that is registered for
  // |mime_type|.
  void OnIsInternalPluginAvailableForMimeType(
      const std::string& mime_type,
      bool* is_available,
      std::vector<base::string16>* additional_param_names,
      std::vector<base::string16>* additional_param_values);
#endif

#if defined(OS_ANDROID)
  void OnSubFrameCreated(int parent_render_frame_id, int child_render_frame_id);
#endif
  ~XWalkRenderMessageFilter() override {}

#if defined(OS_ANDROID)
  int process_id_;
#endif

  DISALLOW_COPY_AND_ASSIGN(XWalkRenderMessageFilter);
};
}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_RENDER_MESSAGE_FILTER_H_
