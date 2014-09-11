// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/tizen/xwalk_web_contents_view_delegate.h"

#include "components/web_modal/popup_manager.h"
#include "components/web_modal/single_web_contents_dialog_manager.h"
#include "components/web_modal/web_contents_modal_dialog_host.h"
#include "components/web_modal/web_contents_modal_dialog_manager.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "ui/aura/client/screen_position_client.h"
#include "ui/aura/window.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/tizen_setting_handler.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

namespace xwalk {
namespace {

scoped_ptr<RenderViewContextMenuImpl> BuildMenu(
    content::WebContents* web_contents,
    const content::ContextMenuParams& params) {
  scoped_ptr<RenderViewContextMenuImpl> menu;
  content::RenderFrameHost* focused_frame = web_contents->GetFocusedFrame();
  // If the frame tree does not have a focused frame at this point, do not
  // bother creating RenderViewContextMenuViews.
  // This happens if the frame has navigated to a different page before
  // ContextMenu message was received by the current RenderFrameHost.
  if (focused_frame) {
    menu.reset(new RenderViewContextMenuImpl(focused_frame, params));
    menu->Init();
  }
  return menu.Pass();
}
}  // namespace

XWalkWebContentsViewDelegate::XWalkWebContentsViewDelegate(
    content::WebContents* web_contents,
    xwalk::application::ApplicationService* app_service)
    : web_contents_(web_contents),
      app_service_(app_service) {
}

XWalkWebContentsViewDelegate::~XWalkWebContentsViewDelegate() {
}

void XWalkWebContentsViewDelegate::ShowContextMenu(
    content::RenderFrameHost* render_frame_host,
    const content::ContextMenuParams& params) {
  int render_process_id = render_frame_host->GetProcess()->GetID();
  application::Application* app = XWalkRunner::GetInstance()->app_system()->
      application_service()->GetApplicationByRenderHostID(render_process_id);
  if (!app)
    return;

  application::TizenSettingInfo* info =
      static_cast<application::TizenSettingInfo*>(
          app->data()->GetManifestData(
              application_widget_keys::kTizenSettingKey));
  if (info && !info->context_menu_enabled())
    return;

  ShowMenu(BuildMenu(
      content::WebContents::FromRenderFrameHost(render_frame_host), params));
}

content::WebDragDestDelegate*
XWalkWebContentsViewDelegate::GetDragDestDelegate() {
  return NULL;
}

void XWalkWebContentsViewDelegate::StoreFocus() {
}

void XWalkWebContentsViewDelegate::RestoreFocus() {
}

bool XWalkWebContentsViewDelegate::Focus() {
  web_modal::PopupManager* popup_manager =
      web_modal::PopupManager::FromWebContents(web_contents_);
  if (popup_manager)
    popup_manager->WasFocused(web_contents_);
  return false;
}

void XWalkWebContentsViewDelegate::TakeFocus(bool reverse) {
}

void XWalkWebContentsViewDelegate::SizeChanged(const gfx::Size& size) {
}

void* XWalkWebContentsViewDelegate::CreateRenderWidgetHostViewDelegate(
    content::RenderWidgetHost* render_widget_host) {
  return NULL;
}

void XWalkWebContentsViewDelegate::ShowMenu(
    scoped_ptr<RenderViewContextMenuImpl> menu) {
  context_menu_.reset(menu.release());

  if (!context_menu_.get())
    return;

  // Menus need a Widget to work. If we're not the active tab we won't
  // necessarily be in a widget.
  views::Widget* top_level_widget = GetTopLevelWidget();
  if (!top_level_widget)
    return;

  const content::ContextMenuParams& params = context_menu_->params();
  // Don't show empty menus.
  if (context_menu_->menu_model().GetItemCount() == 0)
    return;

  gfx::Point screen_point(params.x, params.y);

  // Convert from target window coordinates to root window coordinates.
  aura::Window* target_window = GetActiveNativeView();
  aura::Window* root_window = target_window->GetRootWindow();
  aura::client::ScreenPositionClient* screen_position_client =
      aura::client::GetScreenPositionClient(root_window);
  if (screen_position_client) {
    screen_position_client->ConvertPointToScreen(target_window,
                                                 &screen_point);
  }
  // Enable recursive tasks on the message loop so we can get updates while
  // the context menu is being displayed.
  base::MessageLoop::ScopedNestableTaskAllower allow(
      base::MessageLoop::current());
  context_menu_->RunMenuAt(
      top_level_widget, screen_point, params.source_type);
}

aura::Window* XWalkWebContentsViewDelegate::GetActiveNativeView() {
  return web_contents_->GetFullscreenRenderWidgetHostView()
      ? web_contents_->GetFullscreenRenderWidgetHostView()
      ->GetNativeView()
      : web_contents_->GetNativeView();
}

views::Widget* XWalkWebContentsViewDelegate::GetTopLevelWidget() {
  return views::Widget::GetTopLevelWidgetForNativeView(GetActiveNativeView());
}

views::FocusManager* XWalkWebContentsViewDelegate::GetFocusManager() {
  views::Widget* toplevel_widget = GetTopLevelWidget();
  return toplevel_widget ? toplevel_widget->GetFocusManager() : NULL;
}

void XWalkWebContentsViewDelegate::SetInitialFocus() {
  if (web_contents_->FocusLocationBarByDefault()) {
    if (web_contents_->GetDelegate())
      web_contents_->GetDelegate()->SetFocusToLocationBar(false);
  } else {
    web_contents_->Focus();
  }
}

}  // namespace xwalk
