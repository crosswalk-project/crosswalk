// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_tizen.h"

#include <set>
#include <string>
#include <vector>

#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/screen_orientation_dispatcher_host.h"
#include "content/public/browser/screen_orientation_provider.h"

#include "xwalk/runtime/browser/runtime_context.h"
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

namespace widget_keys = application_widget_keys;

namespace application {

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
  explicit ScreenOrientationProviderTizen(const base::WeakPtr<Application>& app)
      : app_(app),
        request_id_(0) {
  }

  virtual void LockOrientation(
      int request_id,
      blink::WebScreenOrientationLockType lock) OVERRIDE {
    if (!app_)
      return;
    request_id_ = request_id;
    const std::set<Runtime*>& runtimes = app_->runtimes();
    DCHECK(!runtimes.empty());
    // FIXME: Probably need better alignment with
    // https://w3c.github.io/screen-orientation/#screen-orientation-lock-lifetime
    std::set<Runtime*>::iterator it = runtimes.begin();
    for (; it != runtimes.end(); ++it) {
      NativeAppWindow* window = (*it)->window();
      if (window && window->IsActive()) {
        ToNativeAppWindowTizen(window)->LockOrientation(lock);
        break;
      }
    }
  }

  virtual void UnlockOrientation() OVERRIDE {
    LockOrientation(request_id_, GetDefaultOrientation(app_));
  }

  virtual void OnOrientationChange() OVERRIDE {}

 private:
  base::WeakPtr<Application> app_;
  int request_id_;
};

ApplicationTizen::ApplicationTizen(
    scoped_refptr<ApplicationData> data,
    RuntimeContext* runtime_context)
    : Application(data, runtime_context),
      is_suspended_(false) {
#if defined(USE_OZONE)
  ui::PlatformEventSource::GetInstance()->AddPlatformEventObserver(this);
#endif
  cookie_manager_ = scoped_ptr<CookieManager>(
      new CookieManager(id(), runtime_context_));
}

ApplicationTizen::~ApplicationTizen() {
#if defined(USE_OZONE)
  ui::PlatformEventSource::GetInstance()->RemovePlatformEventObserver(this);
#endif
}

void ApplicationTizen::Hide() {
  DCHECK(!runtimes_.empty());
  std::set<Runtime*>::iterator it = runtimes_.begin();
  for (; it != runtimes_.end(); ++it) {
    if ((*it)->window())
      (*it)->window()->Minimize();
  }
}

void ApplicationTizen::Show() {
  DCHECK(!runtimes_.empty());
  for (Runtime* runtime : runtimes_) {
    if (auto window = runtime->window())
      window->Restore();
  }
}

bool ApplicationTizen::Launch(const LaunchParams& launch_params) {
  if (Application::Launch(launch_params)) {
    DCHECK(web_contents_);
    content::ScreenOrientationProvider *provider =
        new ScreenOrientationProviderTizen(GetWeakPtr());
    web_contents_->GetScreenOrientationDispatcherHost()->SetProvider(provider);

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

void ApplicationTizen::Suspend() {
  if (is_suspended_)
    return;

  DCHECK(render_process_host_);
  render_process_host_->Send(new ViewMsg_SuspendJSEngine(true));

  DCHECK(!runtimes_.empty());
  std::set<Runtime*>::iterator it = runtimes_.begin();
  for (; it != runtimes_.end(); ++it) {
    if ((*it)->web_contents())
      (*it)->web_contents()->WasHidden();
  }
  is_suspended_ = true;
}

void ApplicationTizen::Resume() {
  if (!is_suspended_)
    return;

  DCHECK(render_process_host_);
  render_process_host_->Send(new ViewMsg_SuspendJSEngine(false));

  DCHECK(!runtimes_.empty());
  std::set<Runtime*>::iterator it = runtimes_.begin();
  for (; it != runtimes_.end(); ++it) {
    if ((*it)->web_contents())
      (*it)->web_contents()->WasShown();
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

  for (std::set<xwalk::Runtime*>::iterator it = runtimes_.begin();
      it != runtimes_.end(); ++it) {
    (*it)->web_contents()->GetRenderViewHost()->Send(new ViewMsg_HWKeyPressed(
        (*it)->web_contents()->GetRoutingID(), key_event->key_code()));
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

}  // namespace application
}  // namespace xwalk
