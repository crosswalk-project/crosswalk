// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/android/xwalk_permission_client.h"

#include "content/public/common/url_constants.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/WebKit/public/platform/WebURL.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "url/gurl.h"

namespace xwalk {


XWalkPermissionClient::XWalkPermissionClient(content::RenderFrame* render_frame)
    : content::RenderFrameObserver(render_frame) {
  render_frame->GetWebFrame()->setPermissionClient(this);
}

XWalkPermissionClient::~XWalkPermissionClient() {
}

bool XWalkPermissionClient::allowImage(blink::WebFrame* frame,
                                 bool enabled_per_settings,
                                 const blink::WebURL& image_url) {
  // Implementing setBlockNetworkImages, so allow local scheme images to be
  // loaded.
  if (enabled_per_settings)
    return true;

  // For compatibility, only blacklist network schemes instead of whitelisting.
  const GURL url(image_url);
  return !(url.SchemeIs(content::kHttpScheme) ||
           url.SchemeIs(content::kHttpsScheme) ||
           url.SchemeIs(content::kFtpScheme));
}

}  // namespace xwalk
