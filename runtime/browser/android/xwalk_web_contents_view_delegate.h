// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_CONTENTS_VIEW_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_CONTENTS_VIEW_DELEGATE_H_

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "content/public/common/context_menu_params.h"

namespace xwalk {

class XWalkWebContentsViewDelegate : public content::WebContentsViewDelegate {
 public:
  explicit XWalkWebContentsViewDelegate(content::WebContents* web_contents);
  virtual ~XWalkWebContentsViewDelegate();

  // Overridden from WebContentsViewDelegate:
  virtual void ShowContextMenu(content::RenderFrameHost* render_frame_host,
      const content::ContextMenuParams& params) OVERRIDE;
  virtual content::WebDragDestDelegate* GetDragDestDelegate() OVERRIDE;

 private:
  content::WebContents* web_contents_;
  content::ContextMenuParams params_;

  DISALLOW_COPY_AND_ASSIGN(XWalkWebContentsViewDelegate);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_CONTENTS_VIEW_DELEGATE_H_
