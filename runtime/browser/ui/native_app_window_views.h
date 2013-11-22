// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_VIEWS_H_
#define XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_VIEWS_H_

#include <string>

#include "xwalk/runtime/browser/ui/native_app_window.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/rect.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/widget/widget_observer.h"

#if defined(OS_TIZEN_MOBILE)
#include "xwalk/runtime/browser/tizen/sensor_provider.h"
#endif

namespace views {
class WebView;
}

namespace xwalk {

class TopViewLayout;

class NativeAppWindowViews : public NativeAppWindow,
                             public views::WidgetObserver,
                             public views::WidgetDelegateView {
 public:
  explicit NativeAppWindowViews(const NativeAppWindow::CreateParams& params);
  virtual ~NativeAppWindowViews();

  virtual void Initialize();

  // NativeAppWindow implementation.
  virtual gfx::NativeWindow GetNativeWindow() const OVERRIDE;
  virtual void UpdateIcon(const gfx::Image& icon) OVERRIDE;
  virtual void UpdateTitle(const string16& title) OVERRIDE;
  virtual gfx::Rect GetRestoredBounds() const OVERRIDE;
  virtual gfx::Rect GetBounds() const OVERRIDE;
  virtual void SetBounds(const gfx::Rect& bounds) OVERRIDE;
  virtual void Focus() OVERRIDE;
  virtual void Show() OVERRIDE;
  virtual void Hide() OVERRIDE;
  virtual void Maximize() OVERRIDE;
  virtual void Minimize() OVERRIDE;
  virtual void SetFullscreen(bool fullscreen) OVERRIDE;
  virtual void Restore() OVERRIDE;
  virtual void FlashFrame(bool flash) OVERRIDE;
  virtual void Close() OVERRIDE;
  virtual bool IsActive() const OVERRIDE;
  virtual bool IsMaximized() const OVERRIDE;
  virtual bool IsMinimized() const OVERRIDE;
  virtual bool IsFullscreen() const OVERRIDE;

  virtual views::Widget* GetWidget() OVERRIDE;
  virtual const views::Widget* GetWidget() const OVERRIDE;

 protected:
  TopViewLayout* top_view_layout();

  virtual void ViewHierarchyChanged(
      const ViewHierarchyChangedDetails& details) OVERRIDE;

 private:
  // WidgetDelegate implementation.
  virtual views::View* GetInitiallyFocusedView() OVERRIDE;
  virtual views::View* GetContentsView() OVERRIDE;
  virtual string16 GetWindowTitle() const OVERRIDE;
  virtual void DeleteDelegate() OVERRIDE;
  virtual gfx::ImageSkia GetWindowAppIcon() OVERRIDE;
  virtual gfx::ImageSkia GetWindowIcon() OVERRIDE;
  virtual bool ShouldShowWindowTitle() const OVERRIDE;
  virtual void SaveWindowPlacement(
      const gfx::Rect& bounds, ui::WindowShowState show_state);
  virtual bool GetSavedWindowPlacement(const views::Widget* widget,
      gfx::Rect* bounds, ui::WindowShowState* show_state) const OVERRIDE;
  virtual bool CanResize() const OVERRIDE;
  virtual bool CanMaximize() const OVERRIDE;
#if defined(OS_WIN)
  virtual views::NonClientFrameView* CreateNonClientFrameView(
      views::Widget* widget) OVERRIDE;
#endif
  // views::View implementation.
  virtual void ChildPreferredSizeChanged(views::View* child) OVERRIDE;
  virtual void OnFocus() OVERRIDE;
  virtual gfx::Size GetMaximumSize() OVERRIDE { return maximum_size_; }
  virtual gfx::Size GetMinimumSize() OVERRIDE { return minimum_size_; }

  // views::WidgetObserver implementation.
  virtual void OnWidgetClosing(views::Widget* widget) OVERRIDE;
  virtual void OnWidgetCreated(views::Widget* widget) OVERRIDE;
  virtual void OnWidgetDestroying(views::Widget* widget) OVERRIDE;
  virtual void OnWidgetDestroyed(views::Widget* widget) OVERRIDE;
  virtual void OnWidgetBoundsChanged(
      views::Widget* widget, const gfx::Rect& new_bounds) OVERRIDE;

  NativeAppWindow::CreateParams create_params_;

  NativeAppWindowDelegate* delegate_;
  content::WebContents* web_contents_;

  views::WebView* web_view_;
  views::Widget* window_;
  string16 title_;
  gfx::Image icon_;

  bool is_fullscreen_;
  gfx::Size minimum_size_;
  gfx::Size maximum_size_;
  bool resizable_;

  DISALLOW_COPY_AND_ASSIGN(NativeAppWindowViews);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_VIEWS_H_
