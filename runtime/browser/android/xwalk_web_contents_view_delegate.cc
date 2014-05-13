// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_web_contents_view_delegate.h"

#include "base/command_line.h"
#include "content/public/browser/android/content_view_core.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/context_menu_params.h"

namespace xwalk {

XWalkWebContentsViewDelegate::XWalkWebContentsViewDelegate(
    content::WebContents* web_contents)
    : web_contents_(web_contents) {
}

XWalkWebContentsViewDelegate::~XWalkWebContentsViewDelegate() {
}

void XWalkWebContentsViewDelegate::ShowContextMenu(
    content::RenderFrameHost* render_frame_host,
    const content::ContextMenuParams& params) {
  if (params.is_editable && params.selection_text.empty()) {
    content::ContentViewCore* content_view_core =
        content::ContentViewCore::FromWebContents(web_contents_);
    if (content_view_core) {
      content_view_core->ShowPastePopup(params.selection_start.x(),
                                        params.selection_start.y());
    }
  }
}

content::WebDragDestDelegate*
    XWalkWebContentsViewDelegate::GetDragDestDelegate() {
  return NULL;
}

}  // namespace xwalk
