// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/native_app_window_aura.h"

#include "xwalk/runtime/common/xwalk_notification_types.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "ui/aura/env.h"
#include "ui/aura/root_window.h"
#include "ui/aura/window.h"
#include "ui/gfx/screen.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/test/desktop_test_views_delegate.h"
#include "ui/views/view.h"
#include "ui/views/views_delegate.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#include "ui/views/widget/widget.h"

namespace {
views::ViewsDelegate* g_views_delegate_ = NULL;

// ViewDelegate implementation for aura content shell
class XWalkViewsDelegate : public views::DesktopTestViewsDelegate {
 public:
    XWalkViewsDelegate() : use_transparent_windows_(false) {
  }

  virtual ~XWalkViewsDelegate() {
  }

  void SetUseTransparentWindows(bool transparent) {
    use_transparent_windows_ = transparent;
  }

  // Overridden from views::TestViewsDelegate:
  virtual bool UseTransparentWindows() const OVERRIDE {
    return use_transparent_windows_;
  }

 private:
  bool use_transparent_windows_;

  DISALLOW_COPY_AND_ASSIGN(XWalkViewsDelegate);
};
} // namespace

namespace xwalk {

NativeAppWindowAura::NativeAppWindowAura(
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

NativeAppWindowAura::~NativeAppWindowAura() {
}

gfx::NativeWindow NativeAppWindowAura::GetNativeWindow() const {
  return window_->GetNativeWindow();
}

void NativeAppWindowAura::UpdateIcon(const gfx::Image& icon) {
  icon_ = icon;
  window_->UpdateWindowIcon();
}

void NativeAppWindowAura::UpdateTitle(const string16& title) {
  title_ = title;
  window_->UpdateWindowTitle();
}

gfx::Rect NativeAppWindowAura::GetRestoredBounds() const {
  return window_->GetRestoredBounds();
}

gfx::Rect NativeAppWindowAura::GetBounds() const {
  return window_->GetWindowBoundsInScreen();
}

void NativeAppWindowAura::SetBounds(const gfx::Rect& bounds) {
  window_->SetBounds(bounds);
}

void NativeAppWindowAura::Focus() {
  // window_->Focus();
}

void NativeAppWindowAura::Show() {
  window_->Show();
}

void NativeAppWindowAura::Hide() {
  window_->Hide();
}

void NativeAppWindowAura::Maximize() {
  window_->Maximize();
}

void NativeAppWindowAura::Minimize() {
  window_->Minimize();
}

void NativeAppWindowAura::SetFullscreen(bool fullscreen) {
  if (is_fullscreen_ == fullscreen)
    return;
  is_fullscreen_ = fullscreen;
  window_->SetFullscreen(is_fullscreen_);

  content::NotificationService::current()->Notify(
      xwalk::NOTIFICATION_FULLSCREEN_CHANGED,
      content::Source<NativeAppWindow>(this),
      content::NotificationService::NoDetails());
}

void NativeAppWindowAura::Restore() {
  window_->Restore();
}

void NativeAppWindowAura::FlashFrame(bool flash) {
  window_->FlashFrame(flash);
}

void NativeAppWindowAura::Close() {
  window_->Close();
}

bool NativeAppWindowAura::IsActive() const {
  return window_->IsActive();
}

bool NativeAppWindowAura::IsMaximized() const {
  return window_->IsMaximized();
}

bool NativeAppWindowAura::IsMinimized() const {
  return window_->IsMinimized();
}

bool NativeAppWindowAura::IsFullscreen() const {
  return is_fullscreen_;
}

////////////////////////////////////////////////////////////
// WidgetDelegate implementation
////////////////////////////////////////////////////////////
views::View* NativeAppWindowAura::GetInitiallyFocusedView() {
  return web_view_;
}

views::View* NativeAppWindowAura::GetContentsView() {
  return this;
}

views::Widget* NativeAppWindowAura::GetWidget() {
  return window_;
}

const views::Widget* NativeAppWindowAura::GetWidget() const {
  return window_;
}

string16 NativeAppWindowAura::GetWindowTitle() const {
  return title_;
}

void NativeAppWindowAura::DeleteDelegate() {
  window_->RemoveObserver(this);
  delegate_->OnWindowDestroyed();
  delete this;
}

gfx::ImageSkia NativeAppWindowAura::GetWindowAppIcon() {
  return GetWindowIcon();
}

gfx::ImageSkia NativeAppWindowAura::GetWindowIcon() {
  return *icon_.ToImageSkia();
}

bool NativeAppWindowAura::ShouldShowWindowTitle() const {
  return true;
}

void NativeAppWindowAura::SaveWindowPlacement(const gfx::Rect& bounds,
                                          ui::WindowShowState show_state) {
  // TODO(hmin): views::WidgetDelegate::SaveWindowPlacement(bounds, show_state);
}

bool NativeAppWindowAura::GetSavedWindowPlacement(gfx::Rect* bounds,
    ui::WindowShowState* show_state) const {
  // TODO(hmin): Get the saved window placement.
  return false;
}

bool NativeAppWindowAura::CanResize() const {
  return resizable_ &&
      (maximum_size_.IsEmpty() || minimum_size_ != maximum_size_);
}

bool NativeAppWindowAura::CanMaximize() const {
  return resizable_ && maximum_size_.IsEmpty();
}

views::NonClientFrameView* NativeAppWindowAura::CreateNonClientFrameView(
    views::Widget* widget) {
  // TODO(hmin): Need to return a non-client frame for frameless window.
  // Here just return NULL means using a default one.
  return NULL;
}

////////////////////////////////////////////////////////////
// views::View implementation
////////////////////////////////////////////////////////////
void NativeAppWindowAura::Layout() {
  DCHECK(web_view_);
  web_view_->SetBounds(0, 0, width(), height());
}

void NativeAppWindowAura::ViewHierarchyChanged(
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

void NativeAppWindowAura::OnFocus() {
  web_view_->RequestFocus();
}

////////////////////////////////////////////////////////////
// views::WidgetObserver implementation
////////////////////////////////////////////////////////////
void NativeAppWindowAura::OnWidgetClosing(views::Widget* widget) {
}
void NativeAppWindowAura::OnWidgetCreated(views::Widget* widget) {
}
void NativeAppWindowAura::OnWidgetDestroying(views::Widget* widget) {
}
void NativeAppWindowAura::OnWidgetDestroyed(views::Widget* widget) {
}
void NativeAppWindowAura::OnWidgetBoundsChanged(views::Widget* widget,
    const gfx::Rect& new_bounds) {
}

// static
NativeAppWindow* NativeAppWindow::Create(
    const NativeAppWindow::CreateParams& create_params) {
  return new NativeAppWindowAura(create_params);
}

// static
void NativeAppWindow::Initialize() {
  CHECK(!g_views_delegate_);
  gfx::Screen::SetScreenInstance(
      gfx::SCREEN_TYPE_NATIVE, views::CreateDesktopScreen());
  g_views_delegate_ = new XWalkViewsDelegate();
}

}  // namespace xwalk
