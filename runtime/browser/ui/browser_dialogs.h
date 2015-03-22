// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_BROWSER_DIALOGS_H_
#define XWALK_RUNTIME_BROWSER_UI_BROWSER_DIALOGS_H_

#include "ui/gfx/native_widget_types.h"

namespace content {
class BrowserContext;
class WebContents;
}

namespace ui {
class WebDialogDelegate;
}

namespace xwalk {

// Creates and shows an HTML dialog with the given delegate and context.
// The window is automatically destroyed when it is closed.
// Returns the created window.
//
// Make sure to use the returned window only when you know it is safe
// to do so, i.e. before OnDialogClosed() is called on the delegate.
gfx::NativeWindow ShowWebDialog(gfx::NativeWindow parent,
                                content::BrowserContext* context,
                                ui::WebDialogDelegate* delegate);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_BROWSER_DIALOGS_H_
