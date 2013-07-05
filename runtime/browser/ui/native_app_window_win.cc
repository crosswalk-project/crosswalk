// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/runtime/browser/ui/native_app_window_win.h"

#include "cameo/runtime/common/xwalk_notification_types.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "ui/gfx/icon_util.h"
#include "ui/gfx/path.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/views_delegate.h"
#include "ui/views/widget/native_widget_win.h"
#include "ui/views/widget/widget.h"
#include "ui/views/win/hwnd_util.h"

namespace xwalk {

NativeAppWindowWin::NativeAppWindowWin(
    const NativeAppWindow::CreateParams& create_params)
  : delegate_(create_params.delegate),
    web_contents_(create_params.web_contents),
    web_view_(NULL),
    is_fullscreen_(false),
    minimum_size_(create_params.minimum_size),
    maximum_size_(create_params.maximum_size),
    resizable_(create_params.resizable) {
  window_ = new views::Widget;
  views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW);
  params.delegate = this;
  params.remove_standard_frame = false;
  params.use_system_default_icon = true;
  params.top_level = true;
  window_->Init(params);

  gfx::Rect window_bounds = create_params.bounds;
  window_->SetBounds(window_bounds);
  window_->CenterWindow(window_bounds.size());

  if (create_params.state == ui::SHOW_STATE_FULLSCREEN)
    SetFullscreen(true);

  // TODO(hmin): Need to configure the maximum and minimum size of this window.
  window_->AddObserver(this);
}

NativeAppWindowWin::~NativeAppWindowWin() {
}

gfx::NativeWindow NativeAppWindowWin::GetNativeWindow() const {
  return window_->GetNativeWindow();
}

void NativeAppWindowWin::UpdateIcon(const gfx::Image& icon) {
  icon_ = icon;
  window_->UpdateWindowIcon();
}

void NativeAppWindowWin::UpdateTitle(const string16& title) {
  title_ = title;
  window_->UpdateWindowTitle();
}

gfx::Rect NativeAppWindowWin::GetRestoredBounds() const {
  return window_->GetRestoredBounds();
}

gfx::Rect NativeAppWindowWin::GetBounds() const {
  return window_->GetWindowBoundsInScreen();
}

void NativeAppWindowWin::SetBounds(const gfx::Rect& bounds) {
  window_->SetBounds(bounds);
}

void NativeAppWindowWin::Focus() {
  // window_->Focus();
}

void NativeAppWindowWin::Show() {
  window_->Show();
}

void NativeAppWindowWin::Hide() {
  window_->Hide();
}

void NativeAppWindowWin::Maximize() {
  window_->Maximize();
}

void NativeAppWindowWin::Minimize() {
  window_->Minimize();
}

void NativeAppWindowWin::SetFullscreen(bool fullscreen) {
  if (is_fullscreen_ == fullscreen)
    return;
  is_fullscreen_ = fullscreen;
  window_->SetFullscreen(is_fullscreen_);

  content::NotificationService::current()->Notify(
      xwalk::NOTIFICATION_FULLSCREEN_CHANGED,
      content::Source<NativeAppWindow>(this),
      content::NotificationService::NoDetails());
}

void NativeAppWindowWin::Restore() {
  window_->Restore();
}

void NativeAppWindowWin::FlashFrame(bool flash) {
  window_->FlashFrame(flash);
}

void NativeAppWindowWin::Close() {
  window_->Close();
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

bool NativeAppWindowWin::IsFullscreen() const {
  return is_fullscreen_;
}

////////////////////////////////////////////////////////////
// WidgetDelegate implementation
////////////////////////////////////////////////////////////
views::View* NativeAppWindowWin::GetInitiallyFocusedView() {
  return web_view_;
}

views::View* NativeAppWindowWin::GetContentsView() {
  return this;
}

views::Widget* NativeAppWindowWin::GetWidget() {
  return window_;
}

const views::Widget* NativeAppWindowWin::GetWidget() const {
  return window_;
}

string16 NativeAppWindowWin::GetWindowTitle() const {
  return title_;
}

void NativeAppWindowWin::DeleteDelegate() {
  window_->RemoveObserver(this);
  delegate_->OnWindowDestroyed();
  delete this;
}

gfx::ImageSkia NativeAppWindowWin::GetWindowAppIcon() {
  return GetWindowIcon();
}

gfx::ImageSkia NativeAppWindowWin::GetWindowIcon() {
  return *icon_.ToImageSkia();
}

bool NativeAppWindowWin::ShouldShowWindowTitle() const {
  return true;
}

void NativeAppWindowWin::SaveWindowPlacement(const gfx::Rect& bounds,
                                          ui::WindowShowState show_state) {
  // TODO(hmin): views::WidgetDelegate::SaveWindowPlacement(bounds, show_state);
}

bool NativeAppWindowWin::GetSavedWindowPlacement(gfx::Rect* bounds,
    ui::WindowShowState* show_state) const {
  // TODO(hmin): Get the saved window placement.
  return false;
}

bool NativeAppWindowWin::CanResize() const {
  return resizable_ &&
      (maximum_size_.IsEmpty() || minimum_size_ != maximum_size_);
}

bool NativeAppWindowWin::CanMaximize() const {
  return resizable_ && maximum_size_.IsEmpty();
}

views::NonClientFrameView* NativeAppWindowWin::CreateNonClientFrameView(
    views::Widget* widget) {
  // TODO(hmin): Need to return a non-client frame for frameless window.
  // Here just return NULL means using a default one.
  return NULL;
}

////////////////////////////////////////////////////////////
// views::View implementation
////////////////////////////////////////////////////////////
void NativeAppWindowWin::Layout() {
  DCHECK(web_view_);
  web_view_->SetBounds(0, 0, width(), height());
}

void NativeAppWindowWin::ViewHierarchyChanged(
    bool is_add, views::View *parent, views::View *child) {
  if (is_add && child == this) {
    views::BoxLayout* layout = new views::BoxLayout(
        views::BoxLayout::kVertical, 0, 0, 0);
    SetLayoutManager(layout);

    web_view_ = new views::WebView(NULL);
    web_view_->SetWebContents(web_contents_);
    AddChildView(web_view_);
  }
}

void NativeAppWindowWin::OnFocus() {
  web_view_->RequestFocus();
}

////////////////////////////////////////////////////////////
// views::WidgetObserver implementation
////////////////////////////////////////////////////////////
void NativeAppWindowWin::OnWidgetClosing(views::Widget* widget) {
}
void NativeAppWindowWin::OnWidgetCreated(views::Widget* widget) {
}
void NativeAppWindowWin::OnWidgetDestroying(views::Widget* widget) {
}
void NativeAppWindowWin::OnWidgetDestroyed(views::Widget* widget) {
}
void NativeAppWindowWin::OnWidgetBoundsChanged(views::Widget* widget,
    const gfx::Rect& new_bounds) {
}

// static
NativeAppWindow* NativeAppWindow::Create(
    const NativeAppWindow::CreateParams& create_params) {
  return new NativeAppWindowWin(create_params);
}

}  // namespace xwalk
