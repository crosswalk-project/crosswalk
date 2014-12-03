// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_tizen.h"

#include <set>
#include <string>
#include <vector>

#include "content/browser/renderer_host/media/audio_renderer_host.h"
#include "content/browser/renderer_host/render_process_host_impl.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/screen_orientation_dispatcher_host.h"
#include "content/public/browser/screen_orientation_provider.h"

#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/runtime_url_request_context_getter.h"
#include "xwalk/runtime/browser/ui/native_app_window.h"
#include "xwalk/runtime/browser/ui/native_app_window_tizen.h"
#include "xwalk/runtime/common/xwalk_common_messages.h"

#if defined(USE_OZONE)
#include "content/public/browser/render_view_host.h"
#include "ui/events/event.h"
#include "ui/events/event_constants.h"
#include "ui/events/keycodes/keyboard_codes_posix.h"
#include "ui/events/platform/platform_event_source.h"
#endif

#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/tizen_setting_handler.h"
#include "xwalk/application/common/manifest_handlers/tizen_splash_screen_handler.h"

namespace xwalk {

namespace keys = application_manifest_keys;
namespace widget_keys = application_widget_keys;

namespace application {

const char kDefaultMediaAppClass[] = "player";
namespace {
#if defined(OS_TIZEN_MOBILE)
void ApplyRootWindowParams(XWalkContent* content,
                           NativeAppWindow::CreateParams* params) {
  if (!params->delegate)
    params->delegate = content;
  if (params->bounds.IsEmpty())
    params->bounds = gfx::Rect(0, 0, 840, 600);

  unsigned int fullscreen_options = content->fullscreen_options();
  if (params->state == ui::SHOW_STATE_FULLSCREEN)
    fullscreen_options |= XWalkContent::FULLSCREEN_FOR_LAUNCH;
  else
    fullscreen_options &= ~Runtime::FULLSCREEN_FOR_LAUNCH;
  content->set_fullscreen_options(fullscreen_options);
}

NativeAppWindow* CreateRootWindow(XWalkContent* content,
                                  const NativeAppWindow::CreateParams& params) {
  NativeAppWindow::CreateParams effective_params(params);
  ApplyRootWindowParams(content, &effective_params);
  return NativeAppWindow::Create(effective_params);
}
#endif
}  // namespace

blink::WebScreenOrientationLockType GetDefaultOrientation(
    const base::WeakPtr<Application>& app) {
  TizenSettingInfo* info = static_cast<TizenSettingInfo*>(
    app->data()->GetManifestData(widget_keys::kTizenSettingKey));
  if (!info)
    return blink::WebScreenOrientationLockDefault;
  switch (info->screen_orientation()) {
    case TizenSettingInfo::PORTRAIT:
      return blink::WebScreenOrientationLockPortrait;
    case TizenSettingInfo::LANDSCAPE:
      return blink::WebScreenOrientationLockLandscape;
    case TizenSettingInfo::AUTO:
      return blink::WebScreenOrientationLockAny;
    default:
      NOTREACHED();
      return blink::WebScreenOrientationLockDefault;
  }
}

class ScreenOrientationProviderTizen :
    public content::ScreenOrientationProvider {
 public:
  ScreenOrientationProviderTizen(
      const base::WeakPtr<Application>& app,
      content::ScreenOrientationDispatcherHost* dispatcher)
      : app_(app),
        dispatcher_(dispatcher),
        request_id_(0) {
    DCHECK(dispatcher_);
  }

  virtual void LockOrientation(
      int request_id,
      blink::WebScreenOrientationLockType lock) OVERRIDE {
    if (!app_) {
      dispatcher_->NotifyLockError(
          request_id,
          blink::WebLockOrientationError::WebLockOrientationErrorNotAvailable);
      return;
    }
    request_id_ = request_id;
    const std::vector<XWalkContent*>& pages = app_->pages();
    DCHECK(!pages.empty());
    // FIXME: Probably need better alignment with
    // https://w3c.github.io/screen-orientation/#screen-orientation-lock-lifetime
    for (auto it = pages.begin(); it != pages.end(); ++it) {
      NativeAppWindow* window = (*it)->window();
      if (window && window->IsActive()) {
        ToNativeAppWindowTizen(window)->LockOrientation(lock);
        break;
      }
    }
    dispatcher_->NotifyLockSuccess(request_id);
  }

  virtual void UnlockOrientation() OVERRIDE {
    LockOrientation(request_id_, GetDefaultOrientation(app_));
  }

  virtual void OnOrientationChange() OVERRIDE {}

 private:
  base::WeakPtr<Application> app_;
  content::ScreenOrientationDispatcherHost* dispatcher_;
  int request_id_;
};

ApplicationTizen::ApplicationTizen(
    scoped_refptr<ApplicationData> data,
    XWalkBrowserContext* browser_context)
    : Application(data, browser_context),
#if defined(OS_TIZEN_MOBILE)
      root_window_(NULL),
#endif
      is_suspended_(false) {
#if defined(USE_OZONE)
  ui::PlatformEventSource::GetInstance()->AddPlatformEventObserver(this);
#endif
  cookie_manager_ = scoped_ptr<CookieManager>(
      new CookieManager(id(), browser_context_));
}

ApplicationTizen::~ApplicationTizen() {
#if defined(USE_OZONE)
  ui::PlatformEventSource::GetInstance()->RemovePlatformEventObserver(this);
#endif
}

void ApplicationTizen::Hide() {
  DCHECK(!pages_.empty());
  for (XWalkContent* page : pages_) {
    if (auto window = page->window())
      window->Minimize();
  }
}

void ApplicationTizen::Show() {
  DCHECK(!pages_.empty());
  for (XWalkContent* page : pages_) {
    if (auto window = page->window())
      window->Restore();
  }
}

bool ApplicationTizen::Launch(const LaunchParams& launch_params) {
  if (Application::Launch(launch_params)) {
#if defined(OS_TIZEN_MOBILE)
    if (!pages_.empty()) {
      root_window_ = CreateRootWindow(*(pages_.begin()),
                                      window_show_params_);
      window_show_params_.parent = root_window_->GetNativeWindow();
      root_window_->Show();
    }
#endif
    DCHECK(web_contents_);

    // Get media class of application.
    const Manifest* manifest = data_->GetManifest();
    std::string app_class;
    manifest->GetString(keys::kXWalkMediaAppClass, &app_class);
    if (app_class.empty())
      app_class = kDefaultMediaAppClass;

    // Set an application ID and class, which are needed to tag audio
    // streams in pulseaudio/Murphy.
    scoped_refptr<content::AudioRendererHost> audio_host =
        static_cast<content::RenderProcessHostImpl*>(render_process_host_)
            ->audio_renderer_host();
    if (audio_host.get())
      audio_host->SetMediaStreamProperties(id(), app_class);

    content::ScreenOrientationDispatcherHost* host =
        web_contents_->GetScreenOrientationDispatcherHost();
    content::ScreenOrientationProvider* provider =
        new ScreenOrientationProviderTizen(GetWeakPtr(), host);
    host->SetProvider(provider);

    provider->LockOrientation(0, GetDefaultOrientation(GetWeakPtr()));
    return true;
  }
  return false;
}

base::FilePath ApplicationTizen::GetSplashScreenPath() {
  if (TizenSplashScreenInfo* ss_info = static_cast<TizenSplashScreenInfo*>(
      data()->GetManifestData(widget_keys::kTizenSplashScreenKey))) {
    return data()->path().Append(FILE_PATH_LITERAL(ss_info->src()));
  }
  return base::FilePath();
}

bool ApplicationTizen::CanBeSuspended() const {
  if (TizenSettingInfo* setting = static_cast<TizenSettingInfo*>(
          data()->GetManifestData(widget_keys::kTizenSettingKey))) {
    return !setting->background_support_enabled();
  }
  return true;
}

void ApplicationTizen::Suspend() {
  if (is_suspended_ || !CanBeSuspended())
    return;

  DCHECK(render_process_host_);
  render_process_host_->Send(new ViewMsg_SuspendJSEngine(true));

  DCHECK(!pages_.empty());
  for (XWalkContent* page : pages_) {
    page->web_contents()->WasHidden();
  }
  is_suspended_ = true;
}

void ApplicationTizen::Resume() {
  if (!is_suspended_ || !CanBeSuspended())
    return;

  DCHECK(render_process_host_);
  render_process_host_->Send(new ViewMsg_SuspendJSEngine(false));

  DCHECK(!pages_.empty());
  for (XWalkContent* page : pages_) {
    page->web_contents()->WasShown();
  }
  is_suspended_ = false;
}

#if defined(USE_OZONE)
void ApplicationTizen::WillProcessEvent(const ui::PlatformEvent& event) {}

void ApplicationTizen::DidProcessEvent(
    const ui::PlatformEvent& event) {
  ui::Event* ui_event = static_cast<ui::Event*>(event);
  if (!ui_event->IsKeyEvent() || ui_event->type() != ui::ET_KEY_PRESSED)
    return;

  ui::KeyEvent* key_event = static_cast<ui::KeyEvent*>(ui_event);

  // FIXME: Most Wayland devices don't have similar hardware button for 'back'
  // and 'memu' as Tizen Mobile, even that hardare buttons could be different
  // across different kinds of Wayland platforms.
  // Here use external keyboard button 'Backspace' & 'HOME' to emulate 'back'
  // and 'menu' key. Should change this if there is customized key binding.
  if (key_event->key_code() != ui::VKEY_BACK &&
      key_event->key_code() != ui::VKEY_HOME)
    return;

  TizenSettingInfo* info = static_cast<TizenSettingInfo*>(
      data()->GetManifestData(widget_keys::kTizenSettingKey));
  if (info && !info->hwkey_enabled())
    return;

  for (XWalkContent* page : pages_) {
    page->web_contents()->GetRenderViewHost()->Send(new ViewMsg_HWKeyPressed(
        page->web_contents()->GetRoutingID(), key_event->key_code()));
  }
}
#endif

void ApplicationTizen::RemoveAllCookies() {
  cookie_manager_->RemoveAllCookies();
}

void ApplicationTizen::SetUserAgentString(
    const std::string& user_agent_string) {
  cookie_manager_->SetUserAgentString(render_process_host_, user_agent_string);
}

void ApplicationTizen::OnContentCreated(XWalkContent* content) {
  DCHECK(content);
  Application::OnContentCreated(content);
#if defined(OS_TIZEN_MOBILE)
  if (root_window_ && pages_.size() > 1)
      root_window_->Show();
#endif
}

void ApplicationTizen::OnContentClosed(XWalkContent* content) {
  DCHECK(content);
  Application::OnContentClosed(content);
#if defined(OS_TIZEN_MOBILE)
  if (pages_.empty() && root_window_) {
    root_window_->Close();
    root_window_ = NULL;
  }
#endif
}

}  // namespace application
}  // namespace xwalk
