// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/native_app_window_tizen.h"

#include "xwalk/runtime/browser/ui/top_view_layout_views.h"
#include "xwalk/tizen/mobile/ui/tizen_system_indicator.h"

namespace xwalk {

NativeAppWindowTizen::NativeAppWindowTizen(
    const NativeAppWindow::CreateParams& create_params)
    : NativeAppWindowViews(create_params) {
}

NativeAppWindowTizen::~NativeAppWindowTizen() {
}

void NativeAppWindowTizen::ViewHierarchyChanged(
    const ViewHierarchyChangedDetails& details) {
  if (details.is_add && details.child == this) {
    NativeAppWindowViews::ViewHierarchyChanged(details);
    TizenSystemIndicator* indicator = new TizenSystemIndicator();
    if (indicator->IsConnected()) {
      AddChildView(indicator);
      top_view_layout()->set_top_view(indicator);
    } else {
      delete indicator;
    }
  }
}

}  // namespace xwalk
