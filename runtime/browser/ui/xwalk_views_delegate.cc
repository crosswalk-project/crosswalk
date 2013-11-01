// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(USE_AURA)

#include "grit/xwalk_resources.h"

#include "xwalk/runtime/browser/ui/xwalk_views_delegate.h"
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "ui/views/widget/native_widget_aura.h"

namespace xwalk {

XWalkViewsDelegate::XWalkViewsDelegate() {}

XWalkViewsDelegate::~XWalkViewsDelegate() {}

void XWalkViewsDelegate::OnBeforeWidgetInit(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate) {
  // If we already have a native_widget, we are done here.
  if (params->native_widget)
    return;

  bool use_non_toplevel_window = params->parent
      && params->type != views::Widget::InitParams::TYPE_MENU;

  if (!params->parent && !params->context) {
    views::DesktopNativeWidgetAura* native_widget =
        new views::DesktopNativeWidgetAura(delegate);
    params->native_widget = native_widget;
  } else if (use_non_toplevel_window) {
    params->native_widget = new views::NativeWidgetAura(delegate);
  }
}

#if defined(OS_WIN)
HICON XWalkViewsDelegate::GetDefaultWindowIcon() const {
  return LoadIcon(NULL, MAKEINTRESOURCE(IDR_XWALK_ICON_48));
}
#endif

}  // namespace xwalk

#endif  // defined(USE_AURA)

