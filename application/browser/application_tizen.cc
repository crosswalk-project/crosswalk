// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_tizen.h"

#include <set>
#include <string>
#include <vector>

#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_process_host.h"
#include "content/browser/screen_orientation/screen_orientation_dispatcher_host.h"

#include "xwalk/runtime/browser/ui/native_app_window.h"
#include "xwalk/runtime/browser/ui/native_app_window_tizen.h"
#include "xwalk/runtime/common/xwalk_common_messages.h"

#if defined(USE_OZONE)
#include "content/public/browser/render_view_host.h"
#include "ui/events/event.h"
#include "ui/events/event_constants.h"
#include "ui/events/keycodes/keyboard_codes_posix.h"
#include "ui/events/platform/platform_event_source.h"
#include "xwalk/application/common/manifest_handlers/tizen_setting_handler.h"
#endif

#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace widget_keys = application_widget_keys;

namespace application {

ApplicationTizen::ApplicationTizen(
    scoped_refptr<ApplicationData> data,
    RuntimeContext* runtime_context,
    Application::Observer* observer)
    : Application(data, runtime_context, observer) {
#if defined(USE_OZONE)
  ui::PlatformEventSource::GetInstance()->AddPlatformEventObserver(this);
#endif
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
      (*it)->window()->Hide();
  }
}

bool ApplicationTizen::Launch(const LaunchParams& launch_params) {
  if (Application::Launch(launch_params)) {
    DCHECK(render_process_host_);
    render_process_host_->GetScreenOrientationDispatcherHost()->
        SetProviderForTests(this);
    return true;
  }
  return false;
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

void ApplicationTizen::LockOrientation(
      blink::WebScreenOrientationLockType lock) {
  DCHECK(!runtimes_.empty());
  // FIXME: Probably need better alignment with
  // https://w3c.github.io/screen-orientation/#screen-orientation-lock-lifetime
  std::set<Runtime*>::iterator it = runtimes_.begin();
  for (; it != runtimes_.end(); ++it) {
    NativeAppWindow* window = (*it)->window();
    if (window && window->IsActive()) {
      ToNativeAppWindowTizen(window)->LockOrientation(lock);
      break;
    }
  }
}

void ApplicationTizen::UnlockOrientation() {
  LockOrientation(blink::WebScreenOrientationLockDefault);
}

}  // namespace application
}  // namespace xwalk
