// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/browser_dialogs.h"

#include "ui/views/controls/webview/web_dialog_view.h"
#include "ui/views/widget/widget.h"
#include "xwalk/runtime/browser/ui/webui/xwalk_web_contents_handler.h"

namespace xwalk {

// Declared in browser_dialogs.h so that others don't need to depend on our .h.
// Copied from chrome/browser/ui/views/chrome_web_dialog_view.cc
gfx::NativeWindow ShowWebDialog(gfx::NativeWindow parent,
                                content::BrowserContext* context,
                                ui::WebDialogDelegate* delegate) {
  views::WebDialogView* view =
      new views::WebDialogView(context, delegate, new XWalkWebContentsHandler);

  views::Widget* widget = (parent) ?
      views::Widget::CreateWindowWithParent(view, parent) :
      views::Widget::CreateWindow(view);
  widget->Show();

  return widget->GetNativeWindow();
}

}  // namespace xwalk
