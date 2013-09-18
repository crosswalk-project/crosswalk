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
  // If we already have a native_widget, we are done here.
  if (params->native_widget)
    return;

  bool use_non_toplevel_window = params->parent
      && params->type != views::Widget::InitParams::TYPE_MENU;

  if (!params->parent && !params->context) {
    views::DesktopNativeWidgetAura* native_widget =
        new views::DesktopNativeWidgetAura(delegate);
    params->native_widget = native_widget;

    // In order to avoid not building *_x11.cc file which requires patching
    // Chromium, we do not define DesktopRootWindowHost::Create() in
    // DesktopRootWindowHostXWalk and instead create it manually here.
    // This way ::InitNativeWidget will adopt it and not call the wrong
    // ::Create returning DesktopRootWindowHostX11.
    params->desktop_root_window_host =
        new views::DesktopRootWindowHostXWalk(
                delegate, native_widget, params->bounds);

  } else if (use_non_toplevel_window) {
    params->native_widget = new views::NativeWidgetAura(delegate);
  }
}

}  // namespace xwalk

#endif  // !defined(OS_WIN) && defined(USE_AURA)

