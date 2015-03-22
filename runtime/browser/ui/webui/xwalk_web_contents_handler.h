// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_WEBUI_XWALK_WEB_CONTENTS_HANDLER_H_
#define XWALK_RUNTIME_BROWSER_UI_WEBUI_XWALK_WEB_CONTENTS_HANDLER_H_

#include "base/compiler_specific.h"
#include "ui/web_dialogs/web_dialog_web_contents_delegate.h"

class XWalkWebContentsHandler
    : public ui::WebDialogWebContentsDelegate::WebContentsHandler {
 public:
  XWalkWebContentsHandler();
  ~XWalkWebContentsHandler() override;

  // Overridden from WebDialogWebContentsDelegate::WebContentsHandler:
  content::WebContents* OpenURLFromTab(
      content::BrowserContext* context,
      content::WebContents* source,
      const content::OpenURLParams& params) override;
  void AddNewContents(content::BrowserContext* context,
                              content::WebContents* source,
                              content::WebContents* new_contents,
                              WindowOpenDisposition disposition,
                              const gfx::Rect& initial_pos,
                              bool user_gesture) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkWebContentsHandler);
};

#endif  // XWALK_RUNTIME_BROWSER_UI_WEBUI_XWALK_WEB_CONTENTS_HANDLER_H_
