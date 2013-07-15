// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime.h"

#include <string>
#include <utility>

#include "base/command_line.h"
#include "base/message_loop.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
#include "xwalk/runtime/browser/image_util.h"
#include "xwalk/runtime/browser/media/media_capture_devices_dispatcher.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/runtime_file_select_helper.h"
#include "xwalk/runtime/browser/runtime_registry.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "content/public/browser/color_chooser.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
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
  params.initial_size = gfx::Size(kDefaultWidth, kDefaultHeight);
  WebContents* web_contents = WebContents::Create(params);

  Runtime* runtime = Runtime::CreateFromWebContents(web_contents);
  runtime->LoadURL(url);
  return runtime;
}

// static
Runtime* Runtime::CreateFromWebContents(WebContents* web_contents) {
  return new Runtime(web_contents);
}

Runtime::Runtime(content::WebContents* web_contents)
    : WebContentsObserver(web_contents),
      window_(NULL),
      weak_ptr_factory_(this),
      fullscreen_options_(NO_FULLSCREEN)  {
  web_contents_.reset(web_contents);
  web_contents_->SetDelegate(this);
  runtime_context_ =
      static_cast<RuntimeContext*>(web_contents->GetBrowserContext());

  // Set the app icon if it is passed from command line.
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(switches::kAppIcon)) {
    base::FilePath icon_file =
        command_line->GetSwitchValuePath(switches::kAppIcon);
    app_icon_ = xwalk_utils::LoadImageFromFilePath(icon_file);
  }

  NativeAppWindow::CreateParams params;
  params.delegate = this;
  params.web_contents = web_contents_.get();
  params.bounds = gfx::Rect(0, 0, kDefaultWidth, kDefaultHeight);
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kFullscreen)) {
    params.state = ui::SHOW_STATE_FULLSCREEN;
    fullscreen_options_ |= FULLSCREEN_FOR_LAUNCH;
  }

  InitAppWindow(params);

  registrar_.Add(this,
      content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED,
      content::Source<content::WebContents>(web_contents));

  RuntimeRegistry::Get()->AddRuntime(this);
}


Runtime::~Runtime() {
  RuntimeRegistry::Get()->RemoveRuntime(this);

  // Quit the app once the last Runtime instance is removed.
  if (RuntimeRegistry::Get()->runtimes().empty())
    MessageLoop::current()->PostTask(FROM_HERE, MessageLoop::QuitClosure());
}

void Runtime::InitAppWindow(const NativeAppWindow::CreateParams& params) {
  window_ = NativeAppWindow::Create(params);
  if (!app_icon_.IsEmpty())
    window_->UpdateIcon(app_icon_);
  window_->Show();
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
  Runtime::CreateFromWebContents(new_contents);
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
    int color_chooser_id,
    SkColor color) {
#if defined(OS_WIN) && !defined(USE_AURA)
  // On Windows, only create a color chooser if one doesn't exist, because we
  // can't close the old color chooser dialog.
  if (!color_chooser_.get())
    color_chooser_.reset(content::ColorChooser::Create(color_chooser_id,
                                                       web_contents,
                                                       color));
#elif defined(OS_LINUX) && !defined(USE_AURA)
  if (color_chooser_.get())
    color_chooser_->End();
  color_chooser_.reset(content::ColorChooser::Create(color_chooser_id,
                                                     web_contents,
                                                     color));
#endif
  return color_chooser_.get();
}

void Runtime::DidEndColorChooser() {
  color_chooser_.reset();
}

void Runtime::RunFileChooser(
    content::WebContents* web_contents,
    const content::FileChooserParams& params) {
  RuntimeFileSelectHelper::RunFileChooser(web_contents, params);
}

void Runtime::EnumerateDirectory(content::WebContents* web_contents,
                                 int request_id,
                                 const base::FilePath& path) {
  RuntimeFileSelectHelper::EnumerateDirectory(web_contents, request_id, path);
}

void Runtime::DidUpdateFaviconURL(int32 page_id,
                                  const std::vector<FaviconURL>& candidates) {
  DLOG(INFO) << "Candidates: ";
  for (size_t i = 0; i < candidates.size(); ++i)
    DLOG(INFO) << candidates[i].icon_url.spec();

  if (candidates.empty())
    return;

  // Avoid using any previous downloading.
  weak_ptr_factory_.InvalidateWeakPtrs();

  // We only select the first favicon as the icon of app window.
  FaviconURL favicon = candidates[0];
  // Pass 0 to |image_size| parameter means only returning the first bitmap.
  // See content/public/browser/web_contents.h comments.
  web_contents()->DownloadImage(favicon.icon_url, true, 0 /* image size */,
      base::Bind(&Runtime::DidDownloadFavicon, weak_ptr_factory_.GetWeakPtr()));
}

void Runtime::DidDownloadFavicon(int id,
                                 const GURL& image_url,
                                 int requested_size,
                                 const std::vector<SkBitmap>& bitmaps) {
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

  content::MediaStreamDevices devices;
  // Based on chrome/browser/media/media_stream_devices_controller.cc
  bool microphone_requested =
      (request.audio_type == content::MEDIA_DEVICE_AUDIO_CAPTURE);
  bool webcam_requested =
      (request.video_type == content::MEDIA_DEVICE_VIDEO_CAPTURE);
  if (microphone_requested || webcam_requested) {
    switch (request.request_type) {
      case content::MEDIA_OPEN_DEVICE:
        // For open device request pick the desired device or fall back to the
        // first available of the given type.
        XWalkMediaCaptureDevicesDispatcher::GetInstance()->GetRequestedDevice(
            request.requested_device_id,
            microphone_requested,
            webcam_requested,
            &devices);
        break;
      case content::MEDIA_DEVICE_ACCESS:
      case content::MEDIA_GENERATE_STREAM:
      case content::MEDIA_ENUMERATE_DEVICES:
        // Get the default devices for the request.
        XWalkMediaCaptureDevicesDispatcher::GetInstance()->GetRequestedDevice(
            "",
            microphone_requested,
            webcam_requested,
            &devices);
        break;
    }
  }
  callback.Run(devices, scoped_ptr<content::MediaStreamUI>());
}

}  // namespace xwalk
