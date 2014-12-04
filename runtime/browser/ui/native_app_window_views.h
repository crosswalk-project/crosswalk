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
  gfx::NativeWindow GetNativeWindow() const override;
  void UpdateIcon(const gfx::Image& icon) override;
  void UpdateTitle(const base::string16& title) override;
  gfx::Rect GetRestoredBounds() const override;
  gfx::Rect GetBounds() const override;
  void SetBounds(const gfx::Rect& bounds) override;
  void Focus() override;
  void Show() override;
  void Hide() override;
  void Maximize() override;
  void Minimize() override;
  void SetFullscreen(bool fullscreen) override;
  void Restore() override;
  void FlashFrame(bool flash) override;
  void Close() override;
  bool IsActive() const override;
  bool IsMaximized() const override;
  bool IsMinimized() const override;
  bool IsFullscreen() const override;

  views::Widget* GetWidget() override;
  const views::Widget* GetWidget() const override;

 protected:
  TopViewLayout* top_view_layout();
  const NativeAppWindow::CreateParams& create_params() const {
    return create_params_;
  }

  void ViewHierarchyChanged(
      const ViewHierarchyChangedDetails& details) override;

 private:
  // WidgetDelegate implementation.
  views::View* GetInitiallyFocusedView() override;
  views::View* GetContentsView() override;
  base::string16 GetWindowTitle() const override;
  void DeleteDelegate() override;
  gfx::ImageSkia GetWindowAppIcon() override;
  gfx::ImageSkia GetWindowIcon() override;
  bool ShouldShowWindowTitle() const override;
  void SaveWindowPlacement(
      const gfx::Rect& bounds, ui::WindowShowState show_state) override;
  bool GetSavedWindowPlacement(const views::Widget* widget,
      gfx::Rect* bounds, ui::WindowShowState* show_state) const override;
  bool CanResize() const override;
  bool CanMaximize() const override;
  bool CanMinimize() const override;
#if defined(OS_WIN)
  views::NonClientFrameView* CreateNonClientFrameView(
      views::Widget* widget) override;
#endif
  // views::View implementation.
  void ChildPreferredSizeChanged(views::View* child) override;
  void OnFocus() override;
  gfx::Size GetMaximumSize() const override;
  gfx::Size GetMinimumSize() const override;

  // views::WidgetObserver implementation.
  void OnWidgetClosing(views::Widget* widget) override;
  void OnWidgetCreated(views::Widget* widget) override;
  void OnWidgetDestroying(views::Widget* widget) override;
  void OnWidgetDestroyed(views::Widget* widget) override;
  void OnWidgetBoundsChanged(
      views::Widget* widget, const gfx::Rect& new_bounds) override;

  NativeAppWindow::CreateParams create_params_;

  NativeAppWindowDelegate* delegate_;
  content::WebContents* web_contents_;

  views::WebView* web_view_;
  views::Widget* window_;
  base::string16 title_;
  gfx::Image icon_;

  bool is_fullscreen_;
  gfx::Size minimum_size_;
  gfx::Size maximum_size_;
  bool resizable_;

  DISALLOW_COPY_AND_ASSIGN(NativeAppWindowViews);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_VIEWS_H_
