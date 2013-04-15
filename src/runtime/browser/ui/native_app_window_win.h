// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_WIN_H_
#define CAMEO_SRC_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_WIN_H_

#include <string>

#include "cameo/src/runtime/browser/ui/native_app_window.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/rect.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/widget/widget_observer.h"

namespace views {
class WebView;
class Widget;
}

namespace cameo {

class NativeAppWindowWin : public NativeAppWindow,
                           public views::WidgetObserver,
                           public views::WidgetDelegateView {
 public:
  explicit NativeAppWindowWin(const NativeAppWindow::CreateParams& params);
  virtual ~NativeAppWindowWin();

  // NativeAppWindow implementation.
  virtual gfx::NativeWindow GetNativeWindow() const OVERRIDE;
  virtual void UpdateIcon() OVERRIDE;
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

 protected:
  // WidgetDelegate implementation.
  virtual views::View* GetInitiallyFocusedView() OVERRIDE;
  virtual views::View* GetContentsView() OVERRIDE;
  virtual views::Widget* GetWidget() OVERRIDE;
  virtual const views::Widget* GetWidget() const OVERRIDE;
  virtual string16 GetWindowTitle() const OVERRIDE;
  virtual void DeleteDelegate() OVERRIDE;
  virtual gfx::ImageSkia GetWindowAppIcon() OVERRIDE;
  virtual gfx::ImageSkia GetWindowIcon() OVERRIDE;
  virtual bool ShouldShowWindowTitle() const OVERRIDE;
  virtual void SaveWindowPlacement(
      const gfx::Rect& bounds, ui::WindowShowState show_state);
  virtual bool GetSavedWindowPlacement(
      gfx::Rect* bounds, ui::WindowShowState* show_state) const OVERRIDE;
  virtual bool CanResize() const OVERRIDE;
  virtual bool CanMaximize() const OVERRIDE;
  virtual views::NonClientFrameView* CreateNonClientFrameView(
      views::Widget* widget) OVERRIDE;

  // views::View implementation.
  virtual void Layout() OVERRIDE;
  virtual void ViewHierarchyChanged(
      bool is_add, views::View *parent, views::View *child) OVERRIDE;
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

  // Weak reference of the associated Runtime instance.
  Runtime* runtime_;

  views::WebView* web_view_;
  views::Widget* window_;
  string16 title_;

  bool is_fullscreen_;
  gfx::Size minimum_size_;
  gfx::Size maximum_size_;
  bool resizable_;

  DISALLOW_COPY_AND_ASSIGN(NativeAppWindowWin);
};

}  // namespace cameo

#endif  // CAMEO_SRC_RUNTIME_BROWSER_UI_NATIVE_APP_WINDOW_WIN_H_
