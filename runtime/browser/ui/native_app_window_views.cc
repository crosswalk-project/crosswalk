// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/native_app_window_views.h"

#include <vector>

#include "base/command_line.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/web_contents.h"
#include "grit/xwalk_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/display/screen.h"
#include "ui/views/controls/webview/webview.h"
#include "components/constrained_window/constrained_window_views.h"
#include "components/constrained_window/constrained_window_views_client.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#include "ui/views/widget/widget.h"
#include "xwalk/runtime/browser/image_util.h"
#include "xwalk/runtime/browser/ui/desktop/exclusive_access_bubble_views.h"
#include "xwalk/runtime/browser/ui/top_view_layout_views.h"
#include "xwalk/runtime/browser/ui/xwalk_views_delegate.h"
#include "xwalk/runtime/common/xwalk_notification_types.h"
#include "xwalk/runtime/common/xwalk_switches.h"

#if defined(OS_WIN)
#include "ui/views/window/native_frame_view.h"
#endif

#if defined(OS_LINUX) || defined(OS_WIN)
#include "xwalk/runtime/browser/ui/native_app_window_desktop.h"
#endif

namespace xwalk {

NativeAppWindowViews::NativeAppWindowViews(
    const NativeAppWindow::CreateParams& create_params)
    : web_contents_(create_params.web_contents),
      web_view_(nullptr),
      delegate_(create_params.delegate),
      create_params_(create_params),
      window_(nullptr),
      is_fullscreen_(false),
      minimum_size_(create_params.minimum_size),
      maximum_size_(create_params.maximum_size),
      resizable_(create_params.resizable) {
}

NativeAppWindowViews::~NativeAppWindowViews() {}

// Two-step initialization is needed here so that calls done by views::Widget to
// its delegate during views::Widget::Init() reach the correct implementation --
// e.g. ViewHierarchyChanged().
void NativeAppWindowViews::Initialize() {
  CHECK(!window_);
  window_ = new views::Widget;

  views::Widget::InitParams params;
  params.delegate = this;
  params.remove_standard_frame = false;
  params.use_system_default_icon = true;
  params.show_state = create_params_.state;
  params.parent = create_params_.parent;
  // Fullscreen should have higher priority than window size.
  if (create_params_.state != ui::SHOW_STATE_FULLSCREEN) {
    params.type = views::Widget::InitParams::TYPE_WINDOW;
    params.bounds = create_params_.bounds;
  }
  // Set the app icon if it is passed from command line.
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(switches::kAppIcon)) {
    base::FilePath icon_file =
      command_line->GetSwitchValuePath(switches::kAppIcon);
    icon_ = xwalk_utils::LoadImageFromFilePath(icon_file);
  } else {
    // Otherwise, use the default icon for Crosswalk app.
    ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
    icon_ = rb.GetNativeImageNamed(IDR_XWALK_ICON_48);
  }

  window_->Init(params);
#if !defined(USE_OZONE)
  window_->CenterWindow(create_params_.bounds.size());
#endif

  if (create_params_.state == ui::SHOW_STATE_FULLSCREEN)
    SetFullscreen(true);

  // TODO(hmin): Need to configure the maximum and minimum size of this window.
  window_->AddObserver(this);
}

gfx::NativeWindow NativeAppWindowViews::GetNativeWindow() const {
  return window_->GetNativeWindow();
}

void NativeAppWindowViews::UpdateIcon(const gfx::Image& icon) {
  icon_ = icon;
  window_->UpdateWindowIcon();
}

void NativeAppWindowViews::UpdateTitle(const base::string16& title) {
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

  ExclusiveAccessBubble::Type bubble_type =
      (create_params_.state == ui::SHOW_STATE_FULLSCREEN) ?
      ExclusiveAccessBubble::TYPE_APPLICATION_FULLSCREEN_CLOSE_INSTRUCTION
      : ExclusiveAccessBubble::TYPE_BROWSER_FULLSCREEN_EXIT_INSTRUCTION;

  // Add a popup window to exit full screen mode or end the application.
  if (fullscreen) {
    gfx::NativeWindow parent = web_contents_->GetTopLevelNativeWindow();
    exclusive_access_bubble_.reset(
        new ExclusiveAccessBubbleViews(this, bubble_type, parent));
    exclusive_access_bubble_->UpdateContent(bubble_type);
  } else {
    exclusive_access_bubble_.reset();
  }
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

void NativeAppWindowViews::ExitApplication() {
  delegate_->OnApplicationExitRequested();
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

TopViewLayout* NativeAppWindowViews::top_view_layout() {
  return static_cast<TopViewLayout*>(GetLayoutManager());
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

base::string16 NativeAppWindowViews::GetWindowTitle() const {
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
  const gfx::ImageSkia *result =  icon_.ToImageSkia();

  const std::vector<float> oldSupportedScales =
      gfx::ImageSkia::GetSupportedScales();
  std::vector<float> scale = { 1.0f };
  gfx::ImageSkia::SetSupportedScales(scale);
  // Ensure at least the ImageSkiaRep for the default 1x scale is attached to
  // ImageSkia's storage
  result->EnsureRepsForSupportedScales();
  gfx::ImageSkia::SetSupportedScales(oldSupportedScales);
  return *result;
}

bool NativeAppWindowViews::ShouldShowWindowTitle() const {
  return true;
}

void NativeAppWindowViews::SaveWindowPlacement(const gfx::Rect& bounds,
                                               ui::WindowShowState show_state) {
  // TODO(hmin): views::WidgetDelegate::SaveWindowPlacement(bounds, show_state);
}

bool NativeAppWindowViews::GetSavedWindowPlacement(const views::Widget* widget,
    gfx::Rect* bounds, ui::WindowShowState* show_state) const {
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

bool NativeAppWindowViews::CanMinimize() const {
  return true;
}

#if defined(OS_WIN)
views::NonClientFrameView* NativeAppWindowViews::CreateNonClientFrameView(
    views::Widget* widget) {
  return new views::NativeFrameView(widget);
}
#endif

////////////////////////////////////////////////////////////
// views::View implementation
////////////////////////////////////////////////////////////

void NativeAppWindowViews::ChildPreferredSizeChanged(views::View* child) {
  // We only re-layout when the top view changes its preferred size (and notify
  // us by calling its own function PreferredSizeChanged()). We don't react to
  // changes in preferred size for the content view since we are currently
  // setting its bounds to the maximum size available.
  if (child == top_view_layout()->top_view())
    Layout();
}

void NativeAppWindowViews::ViewHierarchyChanged(
    const ViewHierarchyChangedDetails& details) {
  if (details.is_add && details.child == this) {
    TopViewLayout* layout = new TopViewLayout();
    SetLayoutManager(layout);

    web_view_ = new views::WebView(nullptr);
    web_view_->SetWebContents(web_contents_);
    AddChildView(web_view_);
    layout->set_content_view(web_view_);
  }
}

void NativeAppWindowViews::OnFocus() {
  web_view_->RequestFocus();
}

gfx::Size NativeAppWindowViews::GetMaximumSize() const {
  return maximum_size_;
}

gfx::Size NativeAppWindowViews::GetMinimumSize() const {
  return minimum_size_;
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

bool NativeAppWindowViews::PlatformHandleContextMenu(
    const content::ContextMenuParams& params) {
  return false;
}

// Currently, immersive mode is only available for Chrome OS.
bool NativeAppWindowViews::IsImmersiveModeEnabled() {
  return false;
}

NativeAppWindow* NativeAppWindowViews::GetNativeAppViews() {
  return this;
}

views::Widget* NativeAppWindowViews::GetBubbleAssociatedWidget() {
  return GetWidget();
}

gfx::Rect NativeAppWindowViews::GetTopContainerBoundsInScreen() {
  return gfx::Rect();
}

// static
NativeAppWindow* NativeAppWindow::Create(
    const NativeAppWindow::CreateParams& create_params) {
  NativeAppWindowViews* window;
#if defined(OS_LINUX) || defined(OS_WIN)
  window = new NativeAppWindowDesktop(create_params);
#else
  window = new NativeAppWindowViews(create_params);
#endif
  window->Initialize();
  return window;
}

// static
void NativeAppWindow::Initialize() {
  static std::unique_ptr<views::ViewsDelegate> views_delegate_;
  CHECK(!views::ViewsDelegate::GetInstance());
  display::Screen::SetScreenInstance(views::CreateDesktopScreen());
  views_delegate_.reset(new XWalkViewsDelegate);
}

}  // namespace xwalk
