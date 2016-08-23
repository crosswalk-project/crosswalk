// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime.h"

#include <string>
#include <utility>

#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/non_thread_safe.h"
#include "components/app_modal/javascript_dialog_manager.h"
#include "components/content_settings/core/common/content_settings.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/web_contents.h"
#include "grit/xwalk_resources.h"
#include "net/base/url_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/native_widget_types.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/runtime/browser/image_util.h"
#include "xwalk/runtime/browser/media/media_capture_devices_dispatcher.h"
#include "xwalk/runtime/browser/runtime_file_select_helper.h"
#include "xwalk/runtime/browser/ui/color_chooser.h"
#include "xwalk/runtime/browser/xwalk_autofill_manager.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
#include "xwalk/runtime/browser/xwalk_content_settings.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_notification_types.h"
#include "xwalk/runtime/common/xwalk_switches.h"

#if !defined(OS_ANDROID)
#include "xwalk/runtime/browser/runtime_ui_delegate.h"
#endif

using content::FaviconURL;
using content::WebContents;

namespace xwalk {

// static
Runtime* Runtime::Create(XWalkBrowserContext* browser_context,
                         scoped_refptr<content::SiteInstance> site) {
  WebContents::CreateParams params(browser_context, site);
  params.routing_id = MSG_ROUTING_NONE;
  WebContents* web_contents = WebContents::Create(params);

  return new Runtime(web_contents);
}

Runtime::Runtime(content::WebContents* web_contents)
    : WebContentsObserver(web_contents),
      web_contents_(web_contents),
      fullscreen_options_(NO_FULLSCREEN),
      ui_delegate_(nullptr),
      observer_(nullptr),
      weak_ptr_factory_(this) {
  web_contents_->SetDelegate(this);
#if !defined(OS_ANDROID)
  if (XWalkBrowserContext::GetDefault()->save_form_data())
    xwalk_autofill_manager_.reset(
        new XWalkAutofillManager(web_contents_.get()));
#endif
}

Runtime::~Runtime() {
  if (ui_delegate_)
    ui_delegate_->DeleteDelegate();
}

void Runtime::LoadURL(const GURL& url) {
  content::NavigationController::LoadURLParams params(url);
  params.transition_type = ui::PageTransitionFromInt(
      ui::PAGE_TRANSITION_TYPED |
      ui::PAGE_TRANSITION_FROM_ADDRESS_BAR);
  web_contents_->GetController().LoadURLWithParams(params);
  web_contents_->Focus();
}

void Runtime::Show() {
  if (ui_delegate_)
    ui_delegate_->Show();
}

void Runtime::Close() {
  web_contents_->Close();
}

void Runtime::Back() {
  web_contents_->GetController().GoToOffset(-1);
  web_contents_->Focus();
}

void Runtime::Forward() {
  web_contents_->GetController().GoToOffset(1);
  web_contents_->Focus();
}

void Runtime::Reload() {
  web_contents_->GetController().Reload(false);
  web_contents_->Focus();
}

void Runtime::Stop() {
  web_contents_->Stop();
  web_contents_->Focus();
}

NativeAppWindow* Runtime::window() {
  if (ui_delegate_)
    return ui_delegate_->GetAppWindow();
  return nullptr;
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

void Runtime::EnterFullscreenModeForTab(content::WebContents* web_contents,
                                        const GURL&) {
  fullscreen_options_ |= FULLSCREEN_FOR_TAB;
  if (ui_delegate_)
    ui_delegate_->SetFullscreen(true);
}

void Runtime::ExitFullscreenModeForTab(content::WebContents* web_contents) {
  fullscreen_options_ &= ~FULLSCREEN_FOR_TAB;
  if (ui_delegate_)
    ui_delegate_->SetFullscreen(false);
}

bool Runtime::IsFullscreenForTabOrPending(
    const content::WebContents* web_contents) const {
  return (fullscreen_options_ & FULLSCREEN_FOR_TAB) != 0;
}

blink::WebDisplayMode Runtime::GetDisplayMode(
    const content::WebContents* web_contents) const {
  return (ui_delegate_) ? ui_delegate_->GetDisplayMode()
                        : blink::WebDisplayModeUndefined;
}

void Runtime::RequestToLockMouse(content::WebContents* web_contents,
                                 bool user_gesture,
                                 bool last_unlocked_by_target) {
  web_contents->GotResponseToLockMouseRequest(true);
}

void Runtime::CloseContents(content::WebContents* source) {
  if (ui_delegate_)
    ui_delegate_->Close();

  if (observer_)
    observer_->OnRuntimeClosed(this);
}

void Runtime::RequestApplicationExit() {
  if (observer_)
    observer_->OnApplicationExitRequested(this);
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
    ExitFullscreenModeForTab(source);
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
    const std::string& frame_name,
    const GURL& target_url,
    content::WebContents* new_contents) {
  if (observer_)
    observer_->OnNewRuntimeAdded(new Runtime(new_contents));
  else
    LOG(WARNING) << "New web contents is left unhandled.";
}

void Runtime::DidNavigateMainFramePostCommit(
    content::WebContents* web_contents) {
  if (ui_delegate_)
    ui_delegate_->SetAddressURL(web_contents->GetLastCommittedURL());
}

content::JavaScriptDialogManager* Runtime::GetJavaScriptDialogManager(
    content::WebContents* web_contents) {
#if defined(USE_AURA)
  return app_modal::JavaScriptDialogManager::GetInstance();
#else
  return nullptr;
#endif
}

void Runtime::ActivateContents(content::WebContents* contents) {
  contents->GetRenderViewHost()->GetWidget()->Focus();
}

content::ColorChooser* Runtime::OpenColorChooser(
    content::WebContents* web_contents,
    SkColor initial_color,
    const std::vector<content::ColorSuggestion>& suggestions) {
#if defined(TOOLKIT_VIEWS)
  return xwalk::ShowColorChooser(web_contents, initial_color);
#else
  return WebContentsDelegate::OpenColorChooser(web_contents,
                                               initial_color,
                                               suggestions);
#endif
}

void Runtime::RunFileChooser(
    content::RenderFrameHost* render_frame_host,
    const content::FileChooserParams& params) {
#if defined(USE_AURA) && defined(OS_LINUX) && !defined(USE_GTK_UI)
  NOTIMPLEMENTED();
#else
  RuntimeFileSelectHelper::RunFileChooser(render_frame_host, params);
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
      true,  // Normal cache policy
      base::Bind(
          &Runtime::DidDownloadFavicon, weak_ptr_factory_.GetWeakPtr()));
}

void Runtime::TitleWasSet(content::NavigationEntry* entry, bool explicit_set) {
  if (ui_delegate_)
    ui_delegate_->UpdateTitle(entry->GetTitle());
}

void Runtime::DidNavigateAnyFrame(
    content::RenderFrameHost* render_frame_host,
    const content::LoadCommittedDetails& details,
    const content::FrameNavigateParams& params) {
  DCHECK(thread_checker_.CalledOnValidThread());

  XWalkBrowserContext::FromWebContents(web_contents())
      ->AddVisitedURLs(params.redirects);
}

void Runtime::DidDownloadFavicon(int id,
                                 int http_status_code,
                                 const GURL& image_url,
                                 const std::vector<SkBitmap>& bitmaps,
                                 const std::vector<gfx::Size>& sizes) {
  if (bitmaps.empty())
    return;
  app_icon_ = gfx::Image::CreateFrom1xBitmap(bitmaps[0]);
  if (ui_delegate_)
    ui_delegate_->UpdateIcon(app_icon_);
}

bool Runtime::HandleContextMenu(const content::ContextMenuParams& params) {
  if (ui_delegate_)
    return ui_delegate_->HandleContextMenu(params);
  return false;
}

void Runtime::Observe(int type,
                      const content::NotificationSource& source,
                      const content::NotificationDetails& details) {
}

void Runtime::RequestMediaAccessPermission(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback) {
  XWalkMediaCaptureDevicesDispatcher::RunRequestMediaAccessPermission(
      web_contents, request, callback);
}

bool Runtime::CheckMediaAccessPermission(
    content::WebContents* web_contents,
    const GURL& security_origin,
    content::MediaStreamType type) {
  // Requested by Pepper Flash plugin and mediaDevices.enumerateDevices().
#if defined (OS_ANDROID)
  return false;
#else
  // This function may be called for a media request coming from
  // from WebRTC/mediaDevices. These requests can't be made from HTTP.
  if (security_origin.SchemeIs(url::kHttpScheme) &&
      !net::IsLocalhost(security_origin.host()))
    return false;

  ContentSettingsType content_settings_type =
      type == content::MEDIA_DEVICE_AUDIO_CAPTURE
          ? CONTENT_SETTINGS_TYPE_MEDIASTREAM_MIC
          : CONTENT_SETTINGS_TYPE_MEDIASTREAM_CAMERA;
  ContentSetting content_setting =
      XWalkContentSettings::GetInstance()->GetPermission(
          content_settings_type,
          security_origin,
          web_contents_->GetLastCommittedURL().GetOrigin());
  return content_setting == CONTENT_SETTING_ALLOW;
#endif
}

void Runtime::LoadProgressChanged(content::WebContents* source,
                                  double progress) {
  if (ui_delegate_)
    ui_delegate_->SetLoadProgress(progress);
}

bool Runtime::AddDownloadItem(content::DownloadItem* download_item,
    const content::DownloadTargetCallback& callback,
    const base::FilePath& suggested_path) {
  if (ui_delegate_) {
    return ui_delegate_->AddDownloadItem(download_item, callback,
        suggested_path);
  }
  return false;
}

}  // namespace xwalk
