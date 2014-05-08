// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_tizen.h"

#include <set>
#include <string>
#include <vector>

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/url_utils.h"

#include "xwalk/runtime/browser/ui/native_app_window.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_common_messages.h"

#if defined(USE_OZONE)
#include "base/message_loop/message_pump_ozone.h"
#include "content/public/browser/render_view_host.h"
#include "ui/events/event.h"
#include "ui/events/event_constants.h"
#include "ui/events/keycodes/keyboard_codes_posix.h"
#include "xwalk/application/common/manifest_handlers/tizen_setting_handler.h"
#endif

#include "xwalk/application/browser/application_system_linux.h"
#include "xwalk/application/browser/application_service_provider_linux.h"
#include "xwalk/application/browser/linux/running_applications_manager.h"
#include "xwalk/application/browser/linux/running_application_object.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/navigation_handler.h"

namespace xwalk {

namespace widget_keys = application_widget_keys;

namespace application {

ApplicationTizen::ApplicationTizen(
    scoped_refptr<ApplicationData> data,
    RuntimeContext* runtime_context,
    Application::Observer* observer)
    : Application(data, runtime_context, observer) {
#if defined(USE_OZONE)
  base::MessagePumpOzone::Current()->AddObserver(this);
#endif
}

ApplicationTizen::~ApplicationTizen() {
#if defined(USE_OZONE)
  base::MessagePumpOzone::Current()->RemoveObserver(this);
#endif
}

void ApplicationTizen::Hide() {
  DCHECK(runtimes_.size());
  std::set<Runtime*>::iterator it = runtimes_.begin();
  for (; it != runtimes_.end(); ++it) {
    if ((*it)->window())
      (*it)->window()->Hide();
  }
}

void ApplicationTizen::InitSecurityPolicy() {
  // On Tizen, CSP mode has higher priority, and WARP will be disabled
  // if the application is under CSP mode.
  if (!application_data_->HasCSPDefined()) {
    Application::InitSecurityPolicy();
    return;
  }

  if (application_data_->GetPackageType() != Manifest::TYPE_WGT)
    return;

  // Always enable security mode when under CSP mode.
  is_security_mode_ = true;
  NavigationInfo* info = static_cast<NavigationInfo*>(
      application_data_->GetManifestData(widget_keys::kAllowNavigationKey));
  if (info) {
    const std::vector<std::string>& allowed_list = info->GetAllowedDomains();
    for (std::vector<std::string>::const_iterator it = allowed_list.begin();
         it != allowed_list.end(); ++it) {
      // If the policy start with "*.", like this: *.domain,
      // means that can access to all subdomains for 'domain',
      // otherwise, the host of request url should exactly the same
      // as policy.
      bool subdomains = ((*it).find("*.") == 0);
      std::string host = subdomains ? (*it).substr(2) : (*it);
      AddSecurityPolicy(GURL("http://" + host), subdomains);
      AddSecurityPolicy(GURL("https://" + host), subdomains);
    }
  }
  main_runtime_->GetRenderProcessHost()->Send(
      new ViewMsg_EnableSecurityMode(
          ApplicationData::GetBaseURLFromApplicationId(id()),
          SecurityPolicy::CSP));
}

void ApplicationTizen::OnAddMessageToConsole(content::WebContents* source,
                                             int32 level,
                                             const base::string16& message,
                                             int32 line_no,
                                             const base::string16& source_id) {
  ApplicationSystemLinux* app_system_linux =
      static_cast<ApplicationSystemLinux*>(
          XWalkRunner::GetInstance()->app_system());

  RunningApplicationObject* running_application_object =
      app_system_linux->service_provider()->GetRunningApplicationObject(this);

  if (!running_application_object)
    return;

  // Pass through log level only on WebUI pages to limit console spew.
  int32 resolved_level =
      content::HasWebUIScheme(source->GetLastCommittedURL()) ? level : 0;

  std::string log_message;
  if (resolved_level >= ::logging::GetMinLogLevel()) {
    log_message = base::StringPrintf("[CONSOLE(%d)] \"%s\""
                                     ", source: %s (%d)",
                                     resolved_level,
                                     base::UTF16ToUTF8(message).c_str(),
                                     base::UTF16ToUTF8(source_id).c_str(),
                                     line_no);
  }
  running_application_object->SendLogToLauncher(log_message);
}

#if defined(USE_OZONE)
base::EventStatus ApplicationTizen::WillProcessEvent(
    const base::NativeEvent& event) {
  return base::EVENT_CONTINUE;
}

void ApplicationTizen::DidProcessEvent(
    const base::NativeEvent& event) {
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

}  // namespace application
}  // namespace xwalk
