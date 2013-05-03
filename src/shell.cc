// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/shell.h"

#include "base/auto_reset.h"
#include "base/command_line.h"
#include "base/message_loop.h"
#include "base/path_service.h"
#include "base/stringprintf.h"
#include "base/string_number_conversions.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "content/public/browser/devtools_manager.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/renderer_preferences.h"
#include "content/shell/notify_done_forwarder.h"
#include "content/shell/shell_browser_main_parts.h"
#include "content/shell/shell_content_browser_client.h"
#include "content/shell/shell_devtools_frontend.h"
#include "content/shell/shell_javascript_dialog_manager.h"
#include "content/shell/shell_messages.h"
#include "content/shell/shell_switches.h"
#include "content/shell/webkit_test_controller.h"

// Content area size for newly created windows.
static const int kTestWindowWidth = 800;
static const int kTestWindowHeight = 600;

namespace content {

std::vector<Shell*> Shell::windows_;
base::Callback<void(Shell*)> Shell::shell_created_callback_;

bool Shell::quit_message_loop_ = true;

Shell::Shell(WebContents* web_contents)
    : devtools_frontend_(NULL),
      is_fullscreen_(false),
      window_(NULL),
      url_edit_view_(NULL),
#if defined(OS_WIN) && !defined(USE_AURA)
      default_edit_wnd_proc_(0),
#endif
      headless_(false) {
  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
  if (command_line.HasSwitch(switches::kDumpRenderTree) &&
      !command_line.HasSwitch(switches::kDisableHeadlessForLayoutTests)) {
    headless_ = true;
  }
  registrar_.Add(this, NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED,
      Source<WebContents>(web_contents));
  windows_.push_back(this);

  if (!shell_created_callback_.is_null()) {
    shell_created_callback_.Run(this);
    shell_created_callback_.Reset();
  }
}

Shell::~Shell() {
  PlatformCleanUp();

  for (size_t i = 0; i < windows_.size(); ++i) {
    if (windows_[i] == this) {
      windows_.erase(windows_.begin() + i);
      break;
    }
  }

  if (windows_.empty() && quit_message_loop_)
    MessageLoop::current()->PostTask(FROM_HERE, MessageLoop::QuitClosure());
}

Shell* Shell::CreateShell(WebContents* web_contents) {
  Shell* shell = new Shell(web_contents);
  shell->PlatformCreateWindow(kTestWindowWidth, kTestWindowHeight);

  shell->web_contents_.reset(web_contents);
  web_contents->SetDelegate(shell);

  shell->PlatformSetContents();

  shell->PlatformResizeSubViews();

  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree)) {
    web_contents->GetMutableRendererPrefs()->use_custom_colors = false;
    web_contents->GetRenderViewHost()->SyncRendererPrefs();
  }

  return shell;
}

void Shell::CloseAllWindows() {
  base::AutoReset<bool> auto_reset(&quit_message_loop_, false);
  DevToolsManager::GetInstance()->CloseAllClientHosts();
  std::vector<Shell*> open_windows(windows_);
  for (size_t i = 0; i < open_windows.size(); ++i)
    open_windows[i]->Close();
  MessageLoop::current()->RunUntilIdle();
}

void Shell::SetShellCreatedCallback(
    base::Callback<void(Shell*)> shell_created_callback) {
  DCHECK(shell_created_callback_.is_null());
  shell_created_callback_ = shell_created_callback;
}

Shell* Shell::FromRenderViewHost(RenderViewHost* rvh) {
  for (size_t i = 0; i < windows_.size(); ++i) {
    if (windows_[i]->web_contents() &&
        windows_[i]->web_contents()->GetRenderViewHost() == rvh) {
      return windows_[i];
    }
  }
  return NULL;
}

// static
void Shell::Initialize() {
  PlatformInitialize(gfx::Size(kTestWindowWidth, kTestWindowHeight));
}

Shell* Shell::CreateNewWindow(BrowserContext* browser_context,
                              const GURL& url,
                              SiteInstance* site_instance,
                              int routing_id,
                              const gfx::Size& initial_size) {
  WebContents::CreateParams create_params(browser_context, site_instance);
  create_params.routing_id = routing_id;
  if (!initial_size.IsEmpty())
    create_params.initial_size = initial_size;
  else
    create_params.initial_size = gfx::Size(kTestWindowWidth, kTestWindowHeight);
  WebContents* web_contents = WebContents::Create(create_params);
  Shell* shell = CreateShell(web_contents);
  if (!url.is_empty())
    shell->LoadURL(url);
  return shell;
}

void Shell::LoadURL(const GURL& url) {
  LoadURLForFrame(url, std::string());
}

void Shell::LoadURLForFrame(const GURL& url, const std::string& frame_name) {
  NavigationController::LoadURLParams params(url);
  params.transition_type = PageTransitionFromInt(
      PAGE_TRANSITION_TYPED | PAGE_TRANSITION_FROM_ADDRESS_BAR);
  params.frame_name = frame_name;
  web_contents_->GetController().LoadURLWithParams(params);
  web_contents_->GetView()->Focus();
}

void Shell::GoBackOrForward(int offset) {
  web_contents_->GetController().GoToOffset(offset);
  web_contents_->GetView()->Focus();
}

void Shell::Reload() {
  web_contents_->GetController().Reload(false);
  web_contents_->GetView()->Focus();
}

void Shell::Stop() {
  web_contents_->Stop();
  web_contents_->GetView()->Focus();
}

void Shell::UpdateNavigationControls() {
  int current_index = web_contents_->GetController().GetCurrentEntryIndex();
  int max_index = web_contents_->GetController().GetEntryCount() - 1;

  PlatformEnableUIControl(BACK_BUTTON, current_index > 0);
  PlatformEnableUIControl(FORWARD_BUTTON, current_index < max_index);
  PlatformEnableUIControl(STOP_BUTTON, web_contents_->IsLoading());
}

void Shell::ShowDevTools() {
  if (devtools_frontend_) {
    devtools_frontend_->Focus();
    return;
  }
  devtools_frontend_ = ShellDevToolsFrontend::Show(web_contents());
}

void Shell::CloseDevTools() {
  if (!devtools_frontend_)
    return;
  devtools_frontend_->Close();
  devtools_frontend_ = NULL;
}

gfx::NativeView Shell::GetContentView() {
  if (!web_contents_.get())
    return NULL;
  return web_contents_->GetView()->GetNativeView();
}

WebContents* Shell::OpenURLFromTab(WebContents* source,
                                   const OpenURLParams& params) {
  // The only one we implement for now.
  DCHECK(params.disposition == CURRENT_TAB);
  source->GetController().LoadURL(
      params.url, params.referrer, params.transition, std::string());
  return source;
}

void Shell::LoadingStateChanged(WebContents* source) {
  UpdateNavigationControls();
  PlatformSetIsLoading(source->IsLoading());
}

void Shell::ToggleFullscreenModeForTab(WebContents* web_contents,
                                       bool enter_fullscreen) {
#if defined(OS_ANDROID)
  PlatformToggleFullscreenModeForTab(web_contents, enter_fullscreen);
#endif
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return;
  if (is_fullscreen_ != enter_fullscreen) {
    is_fullscreen_ = enter_fullscreen;
    web_contents->GetRenderViewHost()->WasResized();
  }
}

bool Shell::IsFullscreenForTabOrPending(const WebContents* web_contents) const {
#if defined(OS_ANDROID)
  return PlatformIsFullscreenForTabOrPending(web_contents);
#else
  return is_fullscreen_;
#endif
}

void Shell::RequestToLockMouse(WebContents* web_contents,
                               bool user_gesture,
                               bool last_unlocked_by_target) {
  web_contents->GotResponseToLockMouseRequest(true);
}

void Shell::CloseContents(WebContents* source) {
  Close();
}

bool Shell::CanOverscrollContent() const {
#if defined(USE_AURA)
  return true;
#else
  return false;
#endif
}

void Shell::WebContentsCreated(WebContents* source_contents,
                               int64 source_frame_id,
                               const string16& frame_name,
                               const GURL& target_url,
                               WebContents* new_contents) {
  CreateShell(new_contents);
  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    NotifyDoneForwarder::CreateForWebContents(new_contents);
}

void Shell::DidNavigateMainFramePostCommit(WebContents* web_contents) {
  PlatformSetAddressBarURL(web_contents->GetURL());
}

JavaScriptDialogManager* Shell::GetJavaScriptDialogManager() {
  if (!dialog_manager_.get())
    dialog_manager_.reset(new ShellJavaScriptDialogManager());
  return dialog_manager_.get();
}

bool Shell::AddMessageToConsole(WebContents* source,
                                int32 level,
                                const string16& message,
                                int32 line_no,
                                const string16& source_id) {
  return CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree);
}

void Shell::RendererUnresponsive(WebContents* source) {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return;
  WebKitTestController::Get()->RendererUnresponsive();
}

void Shell::ActivateContents(WebContents* contents) {
  contents->GetRenderViewHost()->Focus();
}

void Shell::DeactivateContents(WebContents* contents) {
  contents->GetRenderViewHost()->Blur();
}

void Shell::Observe(int type,
                    const NotificationSource& source,
                    const NotificationDetails& details) {
  if (type == NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED) {
    std::pair<NavigationEntry*, bool>* title =
        Details<std::pair<NavigationEntry*, bool> >(details).ptr();

    if (title->first) {
      string16 text = title->first->GetTitle();
      PlatformSetTitle(text);
    }
  }
}

}  // namespace content
