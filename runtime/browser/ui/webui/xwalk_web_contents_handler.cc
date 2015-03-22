// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/webui/xwalk_web_contents_handler.h"

#include "ui/web_dialogs/web_dialog_web_contents_delegate.h"
#include "xwalk/runtime/browser/runtime.h"

using content::BrowserContext;
using content::OpenURLParams;
using content::WebContents;

XWalkWebContentsHandler::XWalkWebContentsHandler() {
}

XWalkWebContentsHandler::~XWalkWebContentsHandler() {
}

// Opens a new URL inside |source|. |context| is the browser context that the
// browser should be owned by. |params| contains the URL to open and various
// attributes such as disposition. On return |out_new_contents| contains the
// WebContents the URL is opened in. Returns the web contents opened by the
// browser.
WebContents* XWalkWebContentsHandler::OpenURLFromTab(
    content::BrowserContext* context,
    WebContents* source,
    const OpenURLParams& params) {
  if (!context)
    return NULL;
  return OpenURLFromTab(context, source, params);
}

// Creates a new tab with |new_contents|. |context| is the browser context that
// the browser should be owned by. |source| is the WebContent where the
// operation originated. |disposition| controls how the new tab should be
// opened. |initial_pos| is the position of the window if a new window is
// created.  |user_gesture| is true if the operation was started by a user
// gesture.
void XWalkWebContentsHandler::AddNewContents(
    content::BrowserContext* context,
    WebContents* source,
    WebContents* new_contents,
    WindowOpenDisposition disposition,
    const gfx::Rect& initial_pos,
    bool user_gesture) {
  if (!context)
    return;
  AddNewContents(context, source,
    new_contents, disposition, initial_pos, user_gesture);
}
