// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/native_app_window_views.h"

#include "xwalk/runtime/common/xwalk_notification_types.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/view.h"
#include "ui/views/views_delegate.h"
#include "ui/views/widget/widget.h"
#include "ui/views/window/native_frame_view.h"
#include "xwalk/runtime/browser/ui/top_view_layout_views.h"

#if defined(USE_AURA)
#include "ui/aura/env.h"
#include "ui/aura/root_window.h"
#include "ui/aura/window.h"
#include "ui/gfx/screen.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#include "xwalk/runtime/browser/ui/xwalk_views_delegate.h"
#endif

#if defined(OS_WIN) && !defined(USE_AURA)
#include "ui/gfx/icon_util.h"
#endif

#if defined(OS_TIZEN_MOBILE)
#include "xwalk/runtime/browser/ui/tizen_system_indicator.h"
#endif

namespace xwalk {

NativeAppWindowViews::NativeAppWindowViews(
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
  params.bounds = create_params.bounds;
  params.show_state = create_params.state;
  window_->Init(params);

  window_->CenterWindow(create_params.bounds.size());
  if (create_params.state == ui::SHOW_STATE_FULLSCREEN)
    SetFullscreen(true);

  // TODO(hmin): Need to configure the maximum and minimum size of this window.
  window_->AddObserver(this);
}

NativeAppWindowViews::~NativeAppWindowViews() {
}

gfx::NativeWindow NativeAppWindowViews::GetNativeWindow() const {
  return window_->GetNativeWindow();
}

void NativeAppWindowViews::UpdateIcon(const gfx::Image& icon) {
  icon_ = icon;
  window_->UpdateWindowIcon();
}

void NativeAppWindowViews::UpdateTitle(const string16& title) {
  title_ = title;
  window_->UpdateWindowTitle();
}

gfx::Rect NativeAppWindowViews::GetRestoredBounds() const {
  return window_->GetRestoredBounds();
}

gfx::Rect NativeAppWindowViews::GetBounds() const {
  return window_->GetWindowBoundsInScreen();
}

void NativeAppWindowViews::SetBounds(const gfx::Rect& bounds) {
  window_->SetBounds(bounds);
}

void NativeAppWindowViews::Focus() {
  // window_->Focus();
}

void NativeAppWindowViews::Show() {
  window_->Show();
}

void NativeAppWindowViews::Hide() {
  window_->Hide();
}

void NativeAppWindowViews::Maximize() {
  window_->Maximize();
}

void NativeAppWindowViews::Minimize() {
  window_->Minimize();
}

void NativeAppWindowViews::SetFullscreen(bool fullscreen) {
  if (is_fullscreen_ == fullscreen)
    return;
  is_fullscreen_ = fullscreen;
  window_->SetFullscreen(is_fullscreen_);

  content::NotificationService::current()->Notify(
      xwalk::NOTIFICATION_FULLSCREEN_CHANGED,
      content::Source<NativeAppWindow>(this),
      content::NotificationService::NoDetails());
}

void NativeAppWindowViews::Restore() {
  window_->Restore();
}

void NativeAppWindowViews::FlashFrame(bool flash) {
  window_->FlashFrame(flash);
}

void NativeAppWindowViews::Close() {
  window_->Close();
}

bool NativeAppWindowViews::IsActive() const {
  return window_->IsActive();
}

bool NativeAppWindowViews::IsMaximized() const {
  return window_->IsMaximized();
}

bool NativeAppWindowViews::IsMinimized() const {
  return window_->IsMinimized();
}

bool NativeAppWindowViews::IsFullscreen() const {
  return is_fullscreen_;
}

////////////////////////////////////////////////////////////
// WidgetDelegate implementation
////////////////////////////////////////////////////////////
views::View* NativeAppWindowViews::GetInitiallyFocusedView() {
  return web_view_;
}

views::View* NativeAppWindowViews::GetContentsView() {
  return this;
}

views::Widget* NativeAppWindowViews::GetWidget() {
  return window_;
}

const views::Widget* NativeAppWindowViews::GetWidget() const {
  return window_;
}

string16 NativeAppWindowViews::GetWindowTitle() const {
  return title_;
}

void NativeAppWindowViews::DeleteDelegate() {
  window_->RemoveObserver(this);
  delegate_->OnWindowDestroyed();
  delete this;
}

gfx::ImageSkia NativeAppWindowViews::GetWindowAppIcon() {
  return GetWindowIcon();
}

gfx::ImageSkia NativeAppWindowViews::GetWindowIcon() {
  return *icon_.ToImageSkia();
}

bool NativeAppWindowViews::ShouldShowWindowTitle() const {
  return true;
}

void NativeAppWindowViews::SaveWindowPlacement(const gfx::Rect& bounds,
                                          ui::WindowShowState show_state) {
  // TODO(hmin): views::WidgetDelegate::SaveWindowPlacement(bounds, show_state);
}

bool NativeAppWindowViews::GetSavedWindowPlacement(gfx::Rect* bounds,
    ui::WindowShowState* show_state) const {
  // TODO(hmin): Get the saved window placement.
  return false;
}

bool NativeAppWindowViews::CanResize() const {
  return resizable_ &&
      (maximum_size_.IsEmpty() || minimum_size_ != maximum_size_);
}

bool NativeAppWindowViews::CanMaximize() const {
  return resizable_ && maximum_size_.IsEmpty();
}

views::NonClientFrameView* NativeAppWindowViews::CreateNonClientFrameView(
    views::Widget* widget) {
  return new views::NativeFrameView(widget);
}

////////////////////////////////////////////////////////////
// views::View implementation
////////////////////////////////////////////////////////////

void NativeAppWindowViews::ChildPreferredSizeChanged(views::View* child) {
  // We only re-layout when the top view changes its preferred size (and notify
  // us by calling its own function PreferredSizeChanged()). We don't react to
  // changes in preferred size for the content view since we are currently
  // setting its bounds to the maximum size available.
  TopViewLayout* layout = static_cast<TopViewLayout*>(GetLayoutManager());
  if (child == layout->top_view())
    Layout();
}

void NativeAppWindowViews::ViewHierarchyChanged(
    bool is_add, views::View *parent, views::View *child) {
  if (is_add && child == this) {
    TopViewLayout* layout = new TopViewLayout();
    SetLayoutManager(layout);

    web_view_ = new views::WebView(NULL);
    web_view_->SetWebContents(web_contents_);
    AddChildView(web_view_);
    layout->set_content_view(web_view_);

#if defined(OS_TIZEN_MOBILE)
    TizenSystemIndicator* indicator = new TizenSystemIndicator();
    if (indicator->IsConnected()) {
      AddChildView(indicator);
      layout->set_top_view(indicator);
    } else {
      delete indicator;
      LOG(WARNING) << "Failed to load indicator";
    }
#endif
  }
}

void NativeAppWindowViews::OnFocus() {
  web_view_->RequestFocus();
}

////////////////////////////////////////////////////////////
// views::WidgetObserver implementation
////////////////////////////////////////////////////////////
void NativeAppWindowViews::OnWidgetClosing(views::Widget* widget) {
}
void NativeAppWindowViews::OnWidgetCreated(views::Widget* widget) {
}
void NativeAppWindowViews::OnWidgetDestroying(views::Widget* widget) {
}
void NativeAppWindowViews::OnWidgetDestroyed(views::Widget* widget) {
}
void NativeAppWindowViews::OnWidgetBoundsChanged(views::Widget* widget,
    const gfx::Rect& new_bounds) {
}

// static
NativeAppWindow* NativeAppWindow::Create(
    const NativeAppWindow::CreateParams& create_params) {
  return new NativeAppWindowViews(create_params);
}

// static
void NativeAppWindow::Initialize() {
#if !defined(OS_WIN) && defined(USE_AURA)
  CHECK(!views::ViewsDelegate::views_delegate);
  gfx::Screen::SetScreenInstance(
      gfx::SCREEN_TYPE_NATIVE, views::CreateDesktopScreen());
  views::ViewsDelegate::views_delegate = new XWalkViewsDelegate();
#endif
}

}  // namespace xwalk
