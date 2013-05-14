// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/runtime/browser/runtime.h"

#include <string>

#include "base/command_line.h"
#include "base/message_loop.h"
#include "cameo/src/runtime/browser/cameo_browser_main_parts.h"
#include "cameo/src/runtime/browser/cameo_content_browser_client.h"
#include "cameo/src/runtime/browser/runtime_context.h"
#include "cameo/src/runtime/browser/runtime_registry.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"

using content::WebContents;

namespace cameo {

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

//static
Runtime* Runtime::CreateFromWebContents(WebContents* web_contents) {
  return new Runtime(web_contents);
}

Runtime::Runtime(content::WebContents* web_contents) : window_(NULL) {
  web_contents_.reset(web_contents);
  web_contents_->SetDelegate(this);
  runtime_context_ =
      static_cast<RuntimeContext*>(web_contents->GetBrowserContext());

  NativeAppWindow::CreateParams params;
  params.runtime = this;
  params.bounds = gfx::Rect(0, 0, kDefaultWidth, kDefaultHeight);
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
  window_->Show();
}

gfx::Image Runtime::app_icon() const {
  // TODO: Get the app icon for native app window either from app manifest
  // or from html favicon tag.
  return gfx::Image();
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
  // TODO: add fullscreen mode support for native app window.
}

bool Runtime::IsFullscreenForTabOrPending(
    const content::WebContents* web_contents) const {
  return false;
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

void Runtime::DidNavigateMainFramePostCommit(content::WebContents* web_contents) {
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

}  // namespace cameo
