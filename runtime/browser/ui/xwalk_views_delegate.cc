// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(TOOLKIT_VIEWS)

#include "grit/xwalk_resources.h"

#include "xwalk/runtime/browser/ui/xwalk_views_delegate.h"
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "ui/views/widget/native_widget_aura.h"

namespace xwalk {

XWalkViewsDelegate::XWalkViewsDelegate() {}

XWalkViewsDelegate::~XWalkViewsDelegate() {}

bool XWalkViewsDelegate::GetSavedWindowPlacement(
    const views::Widget* widget, const std::string& window_name,
    gfx::Rect* bounds, ui::WindowShowState* show_state) const {
  return false;
}

views::NonClientFrameView* XWalkViewsDelegate::CreateDefaultNonClientFrameView(
    views::Widget* widget) {
  return NULL;
}

content::WebContents* XWalkViewsDelegate::CreateWebContents(
    content::BrowserContext* browser_context,
    content::SiteInstance* site_instance) {
  return NULL;
}

base::TimeDelta
XWalkViewsDelegate::GetDefaultTextfieldObscuredRevealDuration() {
    return base::TimeDelta();
}

void XWalkViewsDelegate::OnBeforeWidgetInit(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate) {
  // If we already have a native_widget, we don't have to try to come
  // up with one.
  if (params->native_widget)
    return;

  if (params->parent &&
    params->type != views::Widget::InitParams::TYPE_MENU &&
    params->type != views::Widget::InitParams::TYPE_TOOLTIP) {
    params->native_widget = new views::NativeWidgetAura(delegate);
  } else {
    params->native_widget = new views::DesktopNativeWidgetAura(delegate);
  }
}

#if defined(OS_WIN)
HICON XWalkViewsDelegate::GetDefaultWindowIcon() const {
  return LoadIcon(NULL, MAKEINTRESOURCE(IDR_XWALK_ICON_48));
}
HICON XWalkViewsDelegate::GetSmallWindowIcon() const {
  return LoadIcon(NULL, MAKEINTRESOURCE(IDR_XWALK_ICON_48));
}
#elif defined(OS_LINUX) && !defined(OS_CHROMEOS)
gfx::ImageSkia* XWalkViewsDelegate::GetDefaultWindowIcon() const {
  return NULL;
}
#endif
}  // namespace xwalk

#endif  // defined(TOOLKIT_VIEWS)

