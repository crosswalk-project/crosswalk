// Copyright (c) 2012 Intel Corp
// Copyright (c) 2012 The Chromium Authors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell co
// pies of the Software, and to permit persons to whom the Software is furnished
//  to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in al
// l copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IM
// PLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNES
// S FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
//  OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WH
// ETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
//  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef CAMEO_SRC_BROWSER_UI_NATIVE_APP_WINDOW_WIN_H_
#define CAMEO_SRC_BROWSER_UI_NATIVE_APP_WINDOW_WIN_H_

#include <string>

#include "cameo/src/browser/ui/native_app_window.h"

#include "third_party/skia/include/core/SkRegion.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/rect.h"
#include "ui/views/focus/widget_focus_manager.h"
#include "ui/views/widget/widget_delegate.h"

namespace views {
class WebView;
class Widget;
}

namespace cameo {

class NativeToolbarWin;

class NativeAppWindowWin : public NativeAppWindow,
                           public views::WidgetDelegateView {
 public:
  explicit NativeAppWindowWin(Shell* shell,
                              const NativeAppWindow::CreateParams& params);
  virtual ~NativeAppWindowWin();

  NativeToolbarWin* toolbar() { return toolbar_; }
  views::Widget* window() { return window_; }

  // NativeAppWindow implementation.
  virtual void SetFullscreen(bool fullscreen) OVERRIDE;
  virtual void UpdateWindowIcon() OVERRIDE;
  virtual void SetTitle(const std::string& title) OVERRIDE;
  virtual bool IsActive() const OVERRIDE;
  virtual bool IsMaximized() const OVERRIDE;
  virtual bool IsMinimized() const OVERRIDE;
  virtual bool IsFullscreen() const OVERRIDE;
  virtual gfx::Rect GetRestoredBounds() const OVERRIDE;
  virtual gfx::Rect GetBounds() const OVERRIDE;
  virtual void Focus() OVERRIDE;
  virtual void Blur() OVERRIDE;
  virtual void Show() OVERRIDE;
  virtual void Hide() OVERRIDE;
  virtual void ShowInactive() OVERRIDE;
  virtual void Close() OVERRIDE;
  virtual void Maximize() OVERRIDE;
  virtual void Minimize() OVERRIDE;
  virtual void Restore() OVERRIDE;
  virtual void SetBounds(const gfx::Rect& bounds) OVERRIDE;
  virtual void FlashFrame(bool flash) OVERRIDE;
  virtual gfx::Size GetSize() OVERRIDE;

  virtual void AddToolbar() OVERRIDE;
  virtual void SetToolbarButtonEnabled(NativeAppWindow::ButtonType button,
                                       bool enabled) OVERRIDE;
  virtual void SetToolbarUrlEntry(const std::string& url) OVERRIDE;
  virtual void SetToolbarIsLoading(bool loading) OVERRIDE;
  virtual void HandleKeyboardEvent(
      const content::NativeWebKeyboardEvent& event) OVERRIDE;
  virtual gfx::NativeWindow GetNativeWindow() OVERRIDE;

  // WidgetDelegate implementation.
  virtual views::View* GetContentsView() OVERRIDE;
  virtual bool CanMaximize() const OVERRIDE;
  virtual views::Widget* GetWidget() OVERRIDE;
  virtual const views::Widget* GetWidget() const OVERRIDE;
  virtual string16 GetWindowTitle() const OVERRIDE;
  virtual void DeleteDelegate() OVERRIDE;
  virtual views::View* GetInitiallyFocusedView() OVERRIDE;
  virtual gfx::ImageSkia GetWindowAppIcon() OVERRIDE;
  virtual gfx::ImageSkia GetWindowIcon() OVERRIDE;
  virtual bool ShouldShowWindowTitle() const OVERRIDE;
  virtual void SaveWindowPlacement(const gfx::Rect& bounds,
                                   ui::WindowShowState show_state);
 protected:
  // views::View implementation.
  virtual void Layout() OVERRIDE;
  virtual void ViewHierarchyChanged(
      bool is_add, views::View *parent, views::View *child) OVERRIDE;
  virtual gfx::Size GetMinimumSize() OVERRIDE;
  virtual gfx::Size GetMaximumSize() OVERRIDE;
  virtual void OnFocus() OVERRIDE;

 private:
  void OnViewWasResized();

  NativeToolbarWin* toolbar_;
  views::WebView* web_view_;
  views::Widget* window_;
  bool is_fullscreen_;

  std::string title_;

  DISALLOW_COPY_AND_ASSIGN(NativeAppWindowWin);
};

}  // namespace cameo

#endif  // CAMEO_SRC_BROWSER_UI_NATIVE_APP_WINDOW_WIN_H_
