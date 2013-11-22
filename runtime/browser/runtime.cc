// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime.h"

#include <string>
#include <utility>

#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "xwalk/runtime/browser/image_util.h"
#include "xwalk/runtime/browser/media/media_capture_devices_dispatcher.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/runtime_file_select_helper.h"
#include "xwalk/runtime/browser/runtime_registry.h"
#include "xwalk/runtime/browser/ui/color_chooser.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/browser/render_process_host.h"
#include "grit/xwalk_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/image/image_skia.h"

using content::FaviconURL;
using content::WebContents;

namespace xwalk {

namespace {

// The default size for web content area size.
const int kDefaultWidth = 840;
const int kDefaultHeight = 600;

}  // namespace

// static
Runtime* Runtime::Create(RuntimeContext* runtime_context, const GURL& url) {
  WebContents::CreateParams params(runtime_context, NULL);
  params.routing_id = MSG_ROUTING_NONE;
  WebContents* web_contents = WebContents::Create(params);

  Runtime* runtime = new Runtime(web_contents);
  runtime->LoadURL(url);
  return runtime;
}

// static
Runtime* Runtime::CreateWithDefaultWindow(
    RuntimeContext* runtime_context, const GURL& url) {
  Runtime* runtime = Runtime::Create(runtime_context, url);
  runtime->AttachDefaultWindow();
  return runtime;
}

Runtime::Runtime(content::WebContents* web_contents)
    : WebContentsObserver(web_contents),
      window_(NULL),
      weak_ptr_factory_(this),
      fullscreen_options_(NO_FULLSCREEN)  {
  web_contents_.reset(web_contents);
  web_contents_->SetDelegate(this);
  runtime_context_ = RuntimeContext::FromWebContents(web_contents);
  RuntimeRegistry::Get()->AddRuntime(this);
}

Runtime::~Runtime() {
  RuntimeRegistry::Get()->RemoveRuntime(this);

  // Quit the app once the last Runtime instance is removed.
  if (RuntimeRegistry::Get()->runtimes().empty()) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE, base::MessageLoop::QuitClosure());
  }
}

void Runtime::AttachDefaultWindow() {
  NativeAppWindow::CreateParams params;
  params.delegate = this;
  params.web_contents = web_contents_.get();
  params.bounds = gfx::Rect(0, 0, kDefaultWidth, kDefaultHeight);
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kFullscreen)) {
    params.state = ui::SHOW_STATE_FULLSCREEN;
    fullscreen_options_ |= FULLSCREEN_FOR_LAUNCH;
  }
  AttachWindow(params);
}

void Runtime::AttachWindow(const NativeAppWindow::CreateParams& params) {
#if defined(OS_ANDROID)
  NOTIMPLEMENTED();
#else
  CHECK(!window_);
  // Set the app icon if it is passed from command line.
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(switches::kAppIcon)) {
    base::FilePath icon_file =
        command_line->GetSwitchValuePath(switches::kAppIcon);
    app_icon_ = xwalk_utils::LoadImageFromFilePath(icon_file);
  } else {
    // Otherwise, use the default icon for Crosswalk app.
    ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
    app_icon_ = rb.GetNativeImageNamed(IDR_XWALK_ICON_48);
  }

  registrar_.Add(this,
        content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED,
        content::Source<content::WebContents>(web_contents_.get()));

  window_ = NativeAppWindow::Create(params);
  if (!app_icon_.IsEmpty())
    window_->UpdateIcon(app_icon_);
  window_->Show();
#endif
}

void Runtime::LoadURL(const GURL& url) {
  content::NavigationController::LoadURLParams params(url);
  params.transition_type = content::PageTransitionFromInt(
      content::PAGE_TRANSITION_TYPED |
      content::PAGE_TRANSITION_FROM_ADDRESS_BAR);
  web_contents_->GetController().LoadURLWithParams(params);
  web_contents_->GetView()->Focus();
}

void Runtime::Close() {
  delete this;
}

NativeAppWindow* Runtime::window() const {
  return window_;
}

//////////////////////////////////////////////////////
// content::WebContentsDelegate:
//////////////////////////////////////////////////////
content::WebContents* Runtime::OpenURLFromTab(
    content::WebContents* source, const content::OpenURLParams& params) {
  // The only one disposition we would take into consideration.
  DCHECK(params.disposition == CURRENT_TAB);
  source->GetController().LoadURL(
      params.url, params.referrer, params.transition, std::string());
  return source;
}

void Runtime::LoadingStateChanged(content::WebContents* source) {
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
    int64 source_frame_id,
    const string16& frame_name,
    const GURL& target_url,
    content::WebContents* new_contents) {
  Runtime* new_runtime = new Runtime(new_contents);
  new_runtime->AttachDefaultWindow();
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

content::ColorChooser* Runtime::OpenColorChooser(
    content::WebContents* web_contents,
    SkColor initial_color) {
  return xwalk::ShowColorChooser(web_contents, initial_color);
}

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

void Runtime::DidUpdateFaviconURL(int32 page_id,
                                  const std::vector<FaviconURL>& candidates) {
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

  RuntimeRegistry::Get()->RuntimeAppIconChanged(this);
}

void Runtime::Observe(int type,
                      const content::NotificationSource& source,
                      const content::NotificationDetails& details) {
  if (type == content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED) {
    std::pair<content::NavigationEntry*, bool>* title =
        content::Details<std::pair<content::NavigationEntry*, bool> >(
            details).ptr();

    if (title->first) {
      string16 text = title->first->GetTitle();
      window_->UpdateTitle(text);
    }
  }
}

void Runtime::OnWindowDestroyed() {
  Close();
}

void Runtime::RequestMediaAccessPermission(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback) {
  XWalkMediaCaptureDevicesDispatcher::RunRequestMediaAccessPermission(
      web_contents, request, callback);
}

void Runtime::RenderProcessGone(base::TerminationStatus status) {
  content::RenderProcessHost* rph = web_contents_->GetRenderProcessHost();
  VLOG(1) << "RenderProcess id: " << rph->GetID() << " is gone!";

  XWalkContentBrowserClient::Get()->RenderProcessHostGone(rph);
}


}  // namespace xwalk
