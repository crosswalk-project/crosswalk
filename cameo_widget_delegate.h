#ifndef CAMEO_CAMEO_WIDGET_DELEGATE_H_
#define CAMEO_CAMEO_WIDGET_DELEGATE_H_

#include "ui/views/widget/widget_delegate.h"

class CameoWidgetDelegate : public views::WidgetDelegateView {
 public:
  CameoWidgetDelegate();

  // views::WidgetDelegateView implementation.
  virtual string16 GetWindowTitle() const OVERRIDE;
  virtual void WindowClosing() OVERRIDE;

  // Override to make this delegate (which is also a view) the
  // contents view for the widget as well, otherwise a default
  // empty view would be used.
  virtual View* GetContentsView() OVERRIDE { return this; }
};

#endif  // CAMEO_CAMEO_WIDGET_DELEGATE_H_
