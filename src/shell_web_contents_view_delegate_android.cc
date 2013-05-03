// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/shell_web_contents_view_delegate.h"

#include "base/command_line.h"
#include "content/public/browser/android/content_view_core.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/context_menu_params.h"
#include "content/shell/shell_web_contents_view_delegate_creator.h"

namespace content {

WebContentsViewDelegate* CreateShellWebContentsViewDelegate(
    WebContents* web_contents) {
  return new ShellWebContentsViewDelegate(web_contents);
}


ShellWebContentsViewDelegate::ShellWebContentsViewDelegate(
    WebContents* web_contents)
    : web_contents_(web_contents) {
}

ShellWebContentsViewDelegate::~ShellWebContentsViewDelegate() {
}

void ShellWebContentsViewDelegate::ShowContextMenu(
    const ContextMenuParams& params,
    ContextMenuSourceType type) {
  if (params.is_editable && params.selection_text.empty()) {
    content::ContentViewCore* content_view_core =
        web_contents_->GetView()->GetContentNativeView();
    if (content_view_core) {
      content_view_core->ShowPastePopup(params.selection_start.x(),
                                        params.selection_start.y());
    }
  }
}

WebDragDestDelegate* ShellWebContentsViewDelegate::GetDragDestDelegate() {
  return NULL;
}

}  // namespace content
