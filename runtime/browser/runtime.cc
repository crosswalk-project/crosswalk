// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime.h"

#include <string>
#include <utility>

#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/runtime/browser/image_util.h"
#include "xwalk/runtime/browser/media/media_capture_devices_dispatcher.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/runtime_file_select_helper.h"
#include "xwalk/runtime/browser/ui/color_chooser.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_notification_types.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "grit/xwalk_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/native_widget_types.h"

#if defined(OS_TIZEN)
#include "content/public/browser/site_instance.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_service.h"
#endif

#if !defined(OS_ANDROID)
#include "xwalk/runtime/browser/runtime_ui_strategy.h"
#endif

using content::FaviconURL;
using content::WebContents;

namespace xwalk {

// static
Runtime* Runtime::Create(RuntimeContext* runtime_context,
                         Observer* observer,
                         content::SiteInstance* site) {
  WebContents::CreateParams params(runtime_context, site);
  params.routing_id = MSG_ROUTING_NONE;
  WebContents* web_contents = WebContents::Create(params);

  return new Runtime(web_contents, observer);
}

Runtime::Runtime(content::WebContents* web_contents, Observer* observer)
    : WebContentsObserver(web_contents),
      web_contents_(web_contents),
      window_(NULL),
      weak_ptr_factory_(this),
      fullscreen_options_(NO_FULLSCREEN),
      remote_debugging_enabled_(false),
      observer_(observer) {
  web_contents_->SetDelegate(this);
  content::NotificationService::current()->Notify(
       xwalk::NOTIFICATION_RUNTIME_OPENED,
       content::Source<Runtime>(this),
       content::NotificationService::NoDetails());
  if (observer_)
    observer_->OnRuntimeAdded(this);
}

Runtime::~Runtime() {
  content::NotificationService::current()->Notify(
          xwalk::NOTIFICATION_RUNTIME_CLOSED,
          content::Source<Runtime>(this),
          content::NotificationService::NoDetails());
  if (observer_)
    observer_->OnRuntimeRemoved(this);
}

void Runtime::EnableTitleUpdatedNotification() {
  registrar_.Add(this,
                 content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED,
                 content::Source<content::WebContents>(web_contents_.get()));
}

void Runtime::set_app_icon(const gfx::Image& app_icon) {
  app_icon_ = app_icon;
  if (window_ && !app_icon_.IsEmpty())
    window_->UpdateIcon(app_icon_);
}

void Runtime::LoadURL(const GURL& url) {
  content::NavigationController::LoadURLParams params(url);
  params.transition_type = ui::PageTransitionFromInt(
      ui::PAGE_TRANSITION_TYPED |
      ui::PAGE_TRANSITION_FROM_ADDRESS_BAR);
  web_contents_->GetController().LoadURLWithParams(params);
  web_contents_->Focus();
}

void Runtime::Close() {
  if (window_) {
    window_->Close();
    return;
  }
  // Runtime should not free itself on Close but be owned
  // by Application.
  delete this;
}

content::RenderProcessHost* Runtime::GetRenderProcessHost() {
  return web_contents_->GetRenderProcessHost();
}

//////////////////////////////////////////////////////
// content::WebContentsDelegate:
//////////////////////////////////////////////////////
content::WebContents* Runtime::OpenURLFromTab(
    content::WebContents* source, const content::OpenURLParams& params) {
#if defined(OS_ANDROID)
  DCHECK(params.disposition == CURRENT_TAB);
  source->GetController().LoadURL(
      params.url, params.referrer, params.transition, std::string());
#else
  if (params.disposition == CURRENT_TAB) {
    source->GetController().LoadURL(
        params.url, params.referrer, params.transition, std::string());
  } else if (params.disposition == NEW_WINDOW ||
             params.disposition == NEW_POPUP ||
             params.disposition == NEW_FOREGROUND_TAB ||
             params.disposition == NEW_BACKGROUND_TAB) {
    // TODO(xinchao): Excecuting JaveScript code is a temporary solution,
    // need to be implemented by creating a new runtime window instead.
    web_contents()->GetFocusedFrame()->ExecuteJavaScript(
        base::UTF8ToUTF16("window.open('" + params.url.spec() + "')"));
  }
#endif
  return source;
}

void Runtime::LoadingStateChanged(content::WebContents* source,
                                  bool to_different_document) {
}

void Runtime::ToggleFullscreenModeForTab(content::WebContents* web_contents,
                                         bool enter_fullscreen) {
  if (enter_fullscreen)
    fullscreen_options_ |= FULLSCREEN_FOR_TAB;
  else
    fullscreen_options_ &= ~FULLSCREEN_FOR_TAB;

  if (enter_fullscreen) {
    window_->SetFullscreen(true);
  } else if (!fullscreen_options_ & FULLSCREEN_FOR_LAUNCH) {
    window_->SetFullscreen(false);
  }
}

bool Runtime::IsFullscreenForTabOrPending(
    const content::WebContents* web_contents) const {
  return (fullscreen_options_ & FULLSCREEN_FOR_TAB) != 0;
}

void Runtime::RequestToLockMouse(content::WebContents* web_contents,
                                 bool user_gesture,
                                 bool last_unlocked_by_target) {
  web_contents->GotResponseToLockMouseRequest(true);
}

void Runtime::CloseContents(content::WebContents* source) {
  window_->Close();
}

bool Runtime::CanOverscrollContent() const {
  return false;
}

bool Runtime::PreHandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event,
      bool* is_keyboard_shortcut) {
  // Escape exits tabbed fullscreen mode.
  if (event.windowsKeyCode == 27 && IsFullscreenForTabOrPending(source)) {
    ToggleFullscreenModeForTab(source, false);
    return true;
  }
  return false;
}

void Runtime::HandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event) {
}

void Runtime::WebContentsCreated(
    content::WebContents* source_contents,
    int opener_render_frame_id,
    const base::string16& frame_name,
    const GURL& target_url,
    content::WebContents* new_contents) {
  new Runtime(new_contents, observer_);
}

void Runtime::DidNavigateMainFramePostCommit(
    content::WebContents* web_contents) {
}

content::JavaScriptDialogManager* Runtime::GetJavaScriptDialogManager() {
  return NULL;
}

void Runtime::ActivateContents(content::WebContents* contents) {
  contents->GetRenderViewHost()->Focus();
}

void Runtime::DeactivateContents(content::WebContents* contents) {
  contents->GetRenderViewHost()->Blur();
}

#if defined(TOOLKIT_VIEWS)
content::ColorChooser* Runtime::OpenColorChooser(
    content::WebContents* web_contents,
    SkColor initial_color,
    const std::vector<content::ColorSuggestion>& suggestions) {
  return xwalk::ShowColorChooser(web_contents, initial_color);
}
#endif

void Runtime::RunFileChooser(
    content::WebContents* web_contents,
    const content::FileChooserParams& params) {
#if defined(USE_AURA) && defined(OS_LINUX)
  NOTIMPLEMENTED();
#else
  RuntimeFileSelectHelper::RunFileChooser(web_contents, params);
#endif
}

void Runtime::EnumerateDirectory(content::WebContents* web_contents,
                                 int request_id,
                                 const base::FilePath& path) {
#if defined(USE_AURA) && defined(OS_LINUX)
  NOTIMPLEMENTED();
#else
  RuntimeFileSelectHelper::EnumerateDirectory(web_contents, request_id, path);
#endif
}

void Runtime::DidUpdateFaviconURL(const std::vector<FaviconURL>& candidates) {
  DLOG(INFO) << "Candidates: ";
  for (size_t i = 0; i < candidates.size(); ++i)
    DLOG(INFO) << candidates[i].icon_url.spec();

  if (candidates.empty())
    return;

  // Avoid using any previous download.
  weak_ptr_factory_.InvalidateWeakPtrs();

  // We only select the first favicon as the window app icon.
  FaviconURL favicon = candidates[0];
  // Passing 0 as the |image_size| parameter results in only receiving the first
  // bitmap, according to content/public/browser/web_contents.h
  web_contents()->DownloadImage(
      favicon.icon_url,
      true,  // Is a favicon
      0,     // No maximum size
      base::Bind(
          &Runtime::DidDownloadFavicon, weak_ptr_factory_.GetWeakPtr()));
}

void Runtime::DidDownloadFavicon(int id,
                                 int http_status_code,
                                 const GURL& image_url,
                                 const std::vector<SkBitmap>& bitmaps,
                                 const std::vector<gfx::Size>& sizes) {
  if (bitmaps.empty())
    return;
  app_icon_ = gfx::Image::CreateFrom1xBitmap(bitmaps[0]);
  window_->UpdateIcon(app_icon_);
}

void Runtime::Observe(int type,
                      const content::NotificationSource& source,
                      const content::NotificationDetails& details) {
  if (type == content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED) {
    std::pair<content::NavigationEntry*, bool>* title =
        content::Details<std::pair<content::NavigationEntry*, bool> >(
            details).ptr();

    if (title->first) {
      base::string16 text = title->first->GetTitle();
      window_->UpdateTitle(text);
    }
  }
}

void Runtime::OnWindowDestroyed() {
  // Runtime should not free itself on Close but be owned
  // by Application.
  delete this;
}

void Runtime::RequestMediaAccessPermission(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback) {
  XWalkMediaCaptureDevicesDispatcher::RunRequestMediaAccessPermission(
      web_contents, request, callback);
}
}  // namespace xwalk
