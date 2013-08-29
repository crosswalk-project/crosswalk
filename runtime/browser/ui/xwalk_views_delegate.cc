// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(OS_WIN) && defined(USE_AURA)

#include "xwalk/runtime/browser/ui/xwalk_views_delegate.h"

#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "ui/views/widget/native_widget_aura.h"
#include "xwalk/runtime/browser/ui/desktop_root_window_host_xwalk.h"

namespace xwalk {

XWalkViewsDelegate::XWalkViewsDelegate() {}

XWalkViewsDelegate::~XWalkViewsDelegate() {}

void XWalkViewsDelegate::OnBeforeWidgetInit(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate) {
  // If we already have a native_widget, we don't have to try to come
  // up with one.
  if (params->native_widget)
    return;

  if (params->parent && params->type != views::Widget::InitParams::TYPE_MENU) {
    params->native_widget = new views::NativeWidgetAura(delegate);
  } else if (!params->parent && !params->context) {
    views::DesktopNativeWidgetAura* desktop_native_widget =
        new views::DesktopNativeWidgetAura(delegate);
    params->native_widget = desktop_native_widget;
    // Provide our own desktop_root_window_host instead of using one provided
    // by views::DesktopNativeWidgetAura.
    params->desktop_root_window_host = new views::DesktopRootWindowHostXWalk(
        delegate, desktop_native_widget, params->bounds);
  }
}

}  // namespace xwalk

#endif  // !defined(OS_WIN) && defined(USE_AURA)

