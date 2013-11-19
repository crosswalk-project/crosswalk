// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_XWALK_VIEWS_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_UI_XWALK_VIEWS_DELEGATE_H_

#if defined(USE_AURA)

#include <string>
#include "ui/views/views_delegate.h"

namespace xwalk {

// Views delegate implementation for Crosswalk. Controls application-wide
// aspects of Views toolkit system.
class XWalkViewsDelegate : public views::ViewsDelegate {
 public:
  XWalkViewsDelegate();
  virtual ~XWalkViewsDelegate();

  // views::ViewsDelegate implementation.
  virtual void SaveWindowPlacement(const views::Widget* widget,
                                   const std::string& window_name,
                                   const gfx::Rect& bounds,
                                   ui::WindowShowState show_state) OVERRIDE {}
  virtual bool GetSavedWindowPlacement(
      const views::Widget* widget,
      const std::string& window_name,
      gfx::Rect* bounds,
      ui::WindowShowState* show_state) const OVERRIDE { return false; }
  virtual void NotifyAccessibilityEvent(
      views::View* view,
      ui::AccessibilityTypes::Event event_type) OVERRIDE {}
  virtual void NotifyMenuItemFocused(const string16& menu_name,
                                     const string16& menu_item_name,
                                     int item_index,
                                     int item_count,
                                     bool has_submenu) OVERRIDE {}
  virtual views::NonClientFrameView* CreateDefaultNonClientFrameView(
      views::Widget* widget) OVERRIDE { return NULL; }
  virtual bool UseTransparentWindows() const OVERRIDE { return false; }
  virtual void AddRef() OVERRIDE {}
  virtual void ReleaseRef() OVERRIDE {}
  virtual content::WebContents* CreateWebContents(
      content::BrowserContext* browser_context,
      content::SiteInstance* site_instance) OVERRIDE { return NULL; }
  virtual void OnBeforeWidgetInit(
      views::Widget::InitParams* params,
      views::internal::NativeWidgetDelegate* delegate) OVERRIDE;
  virtual base::TimeDelta GetDefaultTextfieldObscuredRevealDuration() OVERRIDE {
    return base::TimeDelta();
  }
#if defined(OS_WIN)
  // Retrieves the default window icon to use for windows if none is specified.
  virtual HICON GetDefaultWindowIcon() const OVERRIDE;
#endif

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkViewsDelegate);
};

}  // namespace xwalk

#endif  // defined(USE_AURA)

#endif  // XWALK_RUNTIME_BROWSER_UI_XWALK_VIEWS_DELEGATE_H_
