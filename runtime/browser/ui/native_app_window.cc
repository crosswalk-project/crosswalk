// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/native_app_window.h"

namespace xwalk {

NativeAppWindow::CreateParams::CreateParams()
  : delegate(NULL),
    web_contents(NULL),
    state(ui::SHOW_STATE_NORMAL),
    resizable(true) {
}

}  // namespace xwalk

