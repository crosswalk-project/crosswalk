#ifndef CAMEO_CAMEO_VIEWS_DELEGATE_H_
#define CAMEO_CAMEO_VIEWS_DELEGATE_H_

#include "ui/views/views_delegate.h"

class CameoViewsDelegate : public views::ViewsDelegate {
 public:
  CameoViewsDelegate();
  virtual ~CameoViewsDelegate();

  // ViewsDelegate implementation.
  virtual void SaveWindowPlacement(const views::Widget* widget,
                                   const std::string& window_name,
                                   const gfx::Rect& bounds,
                                   ui::WindowShowState show_state) {}

  virtual bool GetSavedWindowPlacement(
      const std::string& window_name,
      gfx::Rect* bounds,
      ui::WindowShowState* show_state) const { return false; }

  virtual void NotifyAccessibilityEvent(
      views::View* view,
      ui::AccessibilityTypes::Event event_type) { }

  virtual void NotifyMenuItemFocused(const string16& menu_name,
                                     const string16& menu_item_name,
                                     int item_index,
                                     int item_count,
                                     bool has_submenu) { }

  virtual views::NonClientFrameView* CreateDefaultNonClientFrameView(
      views::Widget* widget) { return NULL; }

  virtual bool UseTransparentWindows() const { return false; };

  virtual void AddRef() {}
  virtual void ReleaseRef() {}

  virtual content::WebContents* CreateWebContents(
      content::BrowserContext* browser_context,
      content::SiteInstance* site_instance) { return NULL; }

  virtual void OnBeforeWidgetInit(
      views::Widget::InitParams* params,
      views::internal::NativeWidgetDelegate* delegate);
};

#endif  // CAMEO_CAMEO_VIEWS_DELEGATE
