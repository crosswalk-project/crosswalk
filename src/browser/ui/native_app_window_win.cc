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

#include "cameo/src/browser/ui/native_app_window_win.h"

#include "base/utf_string_conversions.h"
#include "base/values.h"
#include "base/win/wrapped_window_proc.h"
#include "cameo/src/browser/shell.h"
#include "cameo/src/browser/ui/native_toolbar_win.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "extensions/common/draggable_region.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "ui/base/hit_test.h"
#include "ui/base/win/hwnd_util.h"
#include "ui/gfx/path.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/views_delegate.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/native_widget_win.h"
#include "ui/views/window/native_frame_view.h"

namespace cameo {

namespace {

const int kResizeInsideBoundsSize = 5;
const int kResizeAreaCornerSize = 16;

class NativeAppWindowClientView : public views::ClientView {
 public:
  NativeAppWindowClientView(views::Widget* widget,
                            views::View* contents_view,
                            Shell* shell)
      : views::ClientView(widget, contents_view),
        shell_(shell) {
  }
  virtual ~NativeAppWindowClientView() {}

  virtual bool CanClose() OVERRIDE {
    return true;
  }

 private:
  Shell* shell_;
};

}  // namespace

NativeAppWindowWin::NativeAppWindowWin(
    Shell* shell, const NativeAppWindow::CreateParams& create_params)
  : NativeAppWindow(shell, create_params),
    web_view_(NULL),
    toolbar_(NULL),
    is_fullscreen_(false) {
  window_ = new views::Widget;
  views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW);
  params.delegate = this;
  params.remove_standard_frame = false;
  params.use_system_default_icon = true;
  params.top_level = true;
  window_->Init(params);

  if (!shell_->headless() || shell_->is_devtools())
    AddToolbar();
  window_->Show();

  gfx::Rect window_bounds = gfx::Rect(create_params.size);
  window_->SetBounds(window_bounds);
  window_->CenterWindow(window_bounds.size());

  OnViewWasResized();
}

NativeAppWindowWin::~NativeAppWindowWin() {
}

gfx::NativeWindow NativeAppWindowWin::GetNativeWindow() {
  return window_->GetNativeWindow();
}

void NativeAppWindowWin::SetFullscreen(bool fullscreen) {
  is_fullscreen_ = fullscreen;
  window_->SetFullscreen(fullscreen);
}

bool NativeAppWindowWin::IsFullscreen() const {
  return is_fullscreen_;
}

void NativeAppWindowWin::UpdateWindowIcon() {
  window_->UpdateWindowIcon();
}

void NativeAppWindowWin::Close() {
  window_->Close();
}

void NativeAppWindowWin::Focus() {
  window_->Activate();
}

void NativeAppWindowWin::Blur() {
  window_->Deactivate();
}

void NativeAppWindowWin::Show() {
  window_->Show();
}

void NativeAppWindowWin::Hide() {
  window_->Hide();
}

bool NativeAppWindowWin::IsActive() const {
  return window_->IsActive();
}

bool NativeAppWindowWin::IsMaximized() const {
  return window_->IsMaximized();
}

bool NativeAppWindowWin::IsMinimized() const {
  return window_->IsMinimized();
}

void NativeAppWindowWin::ShowInactive() {
  if (window_->IsVisible())
    return;
  window_->ShowInactive();
}

void NativeAppWindowWin::Maximize() {
  window_->Maximize();
}

void NativeAppWindowWin::Minimize() {
  window_->Minimize();
}

void NativeAppWindowWin::Restore() {
  window_->Restore();
}

gfx::Rect NativeAppWindowWin::GetBounds() const {
  return window_->GetWindowBoundsInScreen();
}

void NativeAppWindowWin::SetBounds(const gfx::Rect& bounds) {
  window_->SetBounds(bounds);
}

gfx::Rect NativeAppWindowWin::GetRestoredBounds() const {
  return window_->GetRestoredBounds();
}

void NativeAppWindowWin::FlashFrame(bool flash) {
  window_->FlashFrame(flash);
}

gfx::Size NativeAppWindowWin::GetSize() {
  return window_->GetWindowBoundsInScreen().size();
}

void NativeAppWindowWin::SetTitle(const std::string& title) {
  title_ = title;
  window_->UpdateWindowTitle();
}

void NativeAppWindowWin::AddToolbar() {
  toolbar_ = new NativeToolbarWin(shell_);
  AddChildViewAt(toolbar_, 0);
}

void NativeAppWindowWin::SetToolbarButtonEnabled(
    NativeAppWindow::ButtonType button, bool enabled) {
  if (toolbar_)
    toolbar_->SetButtonEnabled(button, enabled);
}

void NativeAppWindowWin::SetToolbarUrlEntry(const std::string& url) {
  if (toolbar_)
    toolbar_->SetUrlEntry(url);
}

void NativeAppWindowWin::SetToolbarIsLoading(bool loading) {
  if (toolbar_)
    toolbar_->SetIsLoading(loading);
}

views::View* NativeAppWindowWin::GetContentsView() {
  return this;
}

bool NativeAppWindowWin::CanMaximize() const {
  return true;
}

views::Widget* NativeAppWindowWin::GetWidget() {
  return window_;
}

const views::Widget* NativeAppWindowWin::GetWidget() const {
  return window_;
}

string16 NativeAppWindowWin::GetWindowTitle() const {
  return UTF8ToUTF16(title_);
}

void NativeAppWindowWin::DeleteDelegate() {
  delete shell_;
}

bool NativeAppWindowWin::ShouldShowWindowTitle() const {
  return has_frame();
}

gfx::ImageSkia NativeAppWindowWin::GetWindowAppIcon() {
  return gfx::ImageSkia();
}

gfx::ImageSkia NativeAppWindowWin::GetWindowIcon() {
  return GetWindowAppIcon();
}

views::View* NativeAppWindowWin::GetInitiallyFocusedView() {
  return web_view_;
}

void NativeAppWindowWin::HandleKeyboardEvent(
    const content::NativeWebKeyboardEvent& event) {
  // Any unhandled keyboard/character messages should be defproced.
  // This allows stuff like F10, etc to work correctly.
  DefWindowProc(event.os_event.hwnd, event.os_event.message,
                event.os_event.wParam, event.os_event.lParam);
}

void NativeAppWindowWin::Layout() {
  DCHECK(web_view_);
  if (toolbar_) {
    toolbar_->SetBounds(0, 0, width(), 34);
    web_view_->SetBounds(0, 34, width(), height() - 34);
  } else {
    web_view_->SetBounds(0, 0, width(), height());
  }
  OnViewWasResized();
}

void NativeAppWindowWin::ViewHierarchyChanged(
    bool is_add, views::View *parent, views::View *child) {
  if (is_add && child == this) {
    views::BoxLayout* layout = new views::BoxLayout(
        views::BoxLayout::kVertical, 0, 0, 0);
    SetLayoutManager(layout);

    web_view_ = new views::WebView(NULL);
    web_view_->SetWebContents(shell_->web_contents());
    AddChildView(web_view_);
  }
}

gfx::Size NativeAppWindowWin::GetMinimumSize() {
  return minimum_size_;
}

gfx::Size NativeAppWindowWin::GetMaximumSize() {
  return maximum_size_;
}

void NativeAppWindowWin::OnFocus() {
  web_view_->RequestFocus();
}

void NativeAppWindowWin::SaveWindowPlacement(const gfx::Rect& bounds,
                                          ui::WindowShowState show_state) {
  // views::WidgetDelegate::SaveWindowPlacement(bounds, show_state);
}

void NativeAppWindowWin::OnViewWasResized() {
  DCHECK(window_);
  DCHECK(web_view_);
  gfx::Size sz = web_view_->size();
  int height = sz.height(), width = sz.width();
  gfx::Path path;
  path.addRect(0, 0, width, height);
  SetWindowRgn(shell_->web_contents()->GetView()->GetNativeView(),
               path.CreateNativeRegion(), 1);
}

// static
NativeAppWindow* NativeAppWindow::Create(
  Shell* shell, const NativeAppWindow::CreateParams& create_params) {
  return new NativeAppWindowWin(shell, create_params);
}

}  // namespace cameo
