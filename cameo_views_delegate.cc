#include "cameo/cameo_views_delegate.h"

#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "ui/views/widget/native_widget_aura.h"

CameoViewsDelegate::CameoViewsDelegate() {
  ViewsDelegate::views_delegate = this;
}

CameoViewsDelegate::~CameoViewsDelegate() {
  ViewsDelegate::views_delegate = NULL;
}

void CameoViewsDelegate::OnBeforeWidgetInit(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate) {
  // NOTE: from desktop_test_views_delegate.cc:

  // If we already have a native_widget, we don't have to try to come
  // up with one.
  if (params->native_widget)
    return;

  if (params->parent && params->type != views::Widget::InitParams::TYPE_MENU) {
    params->native_widget = new views::NativeWidgetAura(delegate);
  } else if (!params->parent && !params->context) {
    params->native_widget = new views::DesktopNativeWidgetAura(delegate);
  }
}
