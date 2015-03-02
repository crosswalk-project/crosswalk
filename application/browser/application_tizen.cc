// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_tizen.h"

#include <set>
#include <string>
#include <vector>

#include "base/logging.h"

#include "content/browser/renderer_host/media/audio_renderer_host.h"
#include "content/browser/renderer_host/render_process_host_impl.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/screen_orientation_dispatcher_host.h"
#include "content/public/browser/screen_orientation_delegate.h"
#include "content/public/browser/screen_orientation_provider.h"

#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/runtime_url_request_context_getter.h"
#include "xwalk/runtime/browser/ui/native_app_window.h"
#include "xwalk/runtime/browser/ui/native_app_window_tizen.h"
#include "xwalk/runtime/common/xwalk_common_messages.h"

#include "content/public/browser/render_view_host.h"
#include "ui/events/event.h"
#include "ui/events/event_constants.h"
#include "ui/events/event_utils.h"
#include "ui/events/keycodes/keyboard_codes_posix.h"
#include "ui/events/platform/platform_event_source.h"

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
void ApplyRootWindowParams(Runtime* runtime,
                           NativeAppWindow::CreateParams* params) {
  if (!params->delegate)
    params->delegate = runtime;
  if (params->bounds.IsEmpty())
    params->bounds = gfx::Rect(0, 0, 840, 600);

  unsigned int fullscreen_options = runtime->fullscreen_options();
  if (params->state == ui::SHOW_STATE_FULLSCREEN)
    fullscreen_options |= Runtime::FULLSCREEN_FOR_LAUNCH;
  else
    fullscreen_options &= ~Runtime::FULLSCREEN_FOR_LAUNCH;
  runtime->set_fullscreen_options(fullscreen_options);
}

NativeAppWindow* CreateRootWindow(Runtime* runtime,
                                  const NativeAppWindow::CreateParams& params) {
  NativeAppWindow::CreateParams effective_params(params);
  ApplyRootWindowParams(runtime, &effective_params);
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

class ScreenOrientationDelegateTizen :
    public content::ScreenOrientationDelegate {
 public:
  ScreenOrientationDelegateTizen(
      const base::WeakPtr<Application>& app,
      content::ScreenOrientationDispatcherHost* dispatcher)
      : app_(app),
        dispatcher_(dispatcher) {
    DCHECK(dispatcher_);
  }

  bool FullScreenRequired(
      content::WebContents* web_contents) override {
    if (!app_) {
      LOG(ERROR) << "Invalid app error";
      return false;
    }
    return app_->IsFullScreenRequired();
  }

  void Lock(content::WebContents* web_contents,
            blink::WebScreenOrientationLockType lock) override {
    if (!app_) {
      LOG(ERROR) << "Invalid app error";
      return;
    }
    const std::vector<Runtime*>& runtimes = app_->runtimes();
    DCHECK(!runtimes.empty());
    // FIXME: Probably need better alignment with
    // https://w3c.github.io/screen-orientation/#screen-orientation-lock-lifetime
    for (auto it = runtimes.begin(); it != runtimes.end(); ++it) {
      NativeAppWindow* window = (*it)->window();
      if (window && window->IsActive()) {
        ToNativeAppWindowTizen(window)->LockOrientation(lock);
        break;
      }
    }
    int request_id = app_->GetRenderProcessHostID();
    dispatcher_->NotifyLockSuccess(request_id);
  }

  bool ScreenOrientationProviderSupported() override {
    return true;
  }

  void Unlock(content::WebContents* web_contents) override {
    Lock(web_contents, GetDefaultOrientation(app_));
  }

 private:
  base::WeakPtr<Application> app_;
  content::ScreenOrientationDispatcherHost* dispatcher_;
};

ApplicationTizen::ApplicationTizen(
    scoped_refptr<ApplicationData> data,
    XWalkBrowserContext* browser_context)
    : Application(data, browser_context),
#if defined(OS_TIZEN_MOBILE)
      root_window_(NULL),
#endif
      is_suspended_(false) {
  ui::PlatformEventSource::GetInstance()->AddPlatformEventObserver(this);
  cookie_manager_ = scoped_ptr<CookieManager>(
      new CookieManager(id(), browser_context_));
}

ApplicationTizen::~ApplicationTizen() {
  ui::PlatformEventSource::GetInstance()->RemovePlatformEventObserver(this);
}

void ApplicationTizen::Hide() {
  DCHECK(!runtimes_.empty());
  for (auto it = runtimes_.begin(); it != runtimes_.end(); ++it) {
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

bool ApplicationTizen::Launch() {
  if (Application::Launch()) {
#if defined(OS_TIZEN_MOBILE)
    if (!runtimes_.empty()) {
      root_window_ = CreateRootWindow(*(runtimes_.begin()),
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
    content::ScreenOrientationDelegate* delegate =
        new ScreenOrientationDelegateTizen(GetWeakPtr(), host);
    content::ScreenOrientationProvider::SetDelegate(delegate);
    delegate->Lock(web_contents_, GetDefaultOrientation(GetWeakPtr()));
    return true;
  }
  return false;
}

GURL ApplicationTizen::GetStartURL(Manifest::Type type) const {
  const std::string& bundle = data_->bundle();
  if (bundle.empty()) {
    return Application::GetStartURL(type);
  }

  auto app_control = AppControlInfo::CreateFromBundle(bundle);
  if (!app_control) {
    return Application::GetStartURL(type);
  }

  GURL app_control_url = GetAppControlStartURL(*app_control);
  if (!app_control_url.is_valid()) {
    return Application::GetStartURL(type);
  }

  return app_control_url;
}

GURL ApplicationTizen::GetAppControlStartURL(
    const AppControlInfo& app_control) const {
  const AppControlInfoList* app_controls =
      static_cast<const AppControlInfoList*>(
          data()->GetManifestData(
              widget_keys::kTizenApplicationAppControlsKey));
  if (app_controls) {
    for (const auto& item : app_controls->controls) {
      if (item.Covers(app_control)) {
        LOG(INFO) << "Start URL by appcontrol: "
                  << " operation: " << item.operation()
                  << " mime: " << item.mime()
                  << " uri: " << item.uri()
                  << " src: " << item.src();
        return data()->GetResourceURL(item.src());
      }
    }
  }

  return GURL();
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

  DCHECK(!runtimes_.empty());
  for (auto it = runtimes_.begin(); it != runtimes_.end(); ++it) {
    if ((*it)->web_contents())
      (*it)->web_contents()->WasHidden();
  }
  is_suspended_ = true;
}

void ApplicationTizen::Resume() {
  if (!is_suspended_ || !CanBeSuspended())
    return;

  DCHECK(render_process_host_);
  render_process_host_->Send(new ViewMsg_SuspendJSEngine(false));

  DCHECK(!runtimes_.empty());
  for (auto it = runtimes_.begin(); it != runtimes_.end(); ++it) {
    if ((*it)->web_contents())
      (*it)->web_contents()->WasShown();
  }
  is_suspended_ = false;
}

void ApplicationTizen::WillProcessEvent(const ui::PlatformEvent& event) {}

void ApplicationTizen::DidProcessEvent(
    const ui::PlatformEvent& event) {
  scoped_ptr<ui::Event> ui_event(ui::EventFromNative(event));
  if (!ui_event ||
      !ui_event->IsKeyEvent() || ui_event->type() != ui::ET_KEY_PRESSED)
    return;

  ui::KeyEvent* key_event = static_cast<ui::KeyEvent*>(ui_event.get());

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

  for (auto it = runtimes_.begin();
      it != runtimes_.end(); ++it) {
    (*it)->web_contents()->GetRenderViewHost()->Send(new ViewMsg_HWKeyPressed(
        (*it)->web_contents()->GetRoutingID(), key_event->key_code()));
  }
}

void ApplicationTizen::RemoveAllCookies() {
  cookie_manager_->RemoveAllCookies();
}

void ApplicationTizen::SetUserAgentString(
    const std::string& user_agent_string) {
  cookie_manager_->SetUserAgentString(render_process_host_, user_agent_string);
}

void ApplicationTizen::OnNewRuntimeAdded(Runtime* runtime) {
  DCHECK(runtime);
  Application::OnNewRuntimeAdded(runtime);
#if defined(OS_TIZEN_MOBILE)
  if (root_window_ && runtimes_.size() > 1)
      root_window_->Show();
#endif
}

void ApplicationTizen::OnRuntimeClosed(Runtime* runtime) {
  DCHECK(runtime);
  Application::OnRuntimeClosed(runtime);
#if defined(OS_TIZEN_MOBILE)
  if (runtimes_.empty() && root_window_) {
    root_window_->Close();
    root_window_ = NULL;
  }
#endif
}

}  // namespace application
}  // namespace xwalk
