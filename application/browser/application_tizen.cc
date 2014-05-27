// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_tizen.h"

#include <set>
#include <string>
#include <vector>

#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_process_host.h"

#include "xwalk/runtime/browser/ui/native_app_window.h"
#include "xwalk/runtime/common/xwalk_common_messages.h"

#if defined(USE_OZONE)
#include "content/public/browser/render_view_host.h"
#include "ui/events/event.h"
#include "ui/events/event_constants.h"
#include "ui/events/keycodes/keyboard_codes_posix.h"
#include "xwalk/application/common/manifest_handlers/tizen_setting_handler.h"
#endif

#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/csp_handler.h"
#include "xwalk/application/common/manifest_handlers/navigation_handler.h"

namespace xwalk {

namespace widget_keys = application_widget_keys;

namespace application {

namespace {
const char kAsterisk[] = "*";

const char kDirectiveValueSelf[] = "'self'";
const char kDirectiveValueNone[] = "'none'";

const char kDirectiveNameDefault[] = "default-src";
const char kDirectiveNameScript[] = "script-src";
const char kDirectiveNameStyle[] = "style-src";
const char kDirectiveNameObject[] = "object-src";

CSPInfo* GetDefaultCSPInfo() {
  static CSPInfo default_csp_info;
  if (default_csp_info.GetDirectives().empty()) {
    std::vector<std::string> directive_all;
    std::vector<std::string> directive_self;
    std::vector<std::string> directive_none;
    directive_all.push_back(kAsterisk);
    directive_self.push_back(kDirectiveValueSelf);
    directive_none.push_back(kDirectiveValueNone);

    default_csp_info.SetDirective(kDirectiveNameDefault, directive_all);
    default_csp_info.SetDirective(kDirectiveNameScript, directive_self);
    default_csp_info.SetDirective(kDirectiveNameStyle, directive_self);
    default_csp_info.SetDirective(kDirectiveNameObject, directive_none);
  }

  return (new CSPInfo(default_csp_info));
}

}  // namespace


ApplicationTizen::ApplicationTizen(
    scoped_refptr<ApplicationData> data,
    RuntimeContext* runtime_context,
    Application::Observer* observer)
    : Application(data, runtime_context, observer) {
}

ApplicationTizen::~ApplicationTizen() {
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
  if (!data_->HasCSPDefined()) {
    Application::InitSecurityPolicy();
    return;
  }

  if (data_->GetPackageType() != Package::WGT)
    return;

  CSPInfo* csp_info =
      static_cast<CSPInfo*>(data_->GetManifestData(widget_keys::kCSPKey));
  if (!csp_info || csp_info->GetDirectives().empty())
    data_->SetManifestData(widget_keys::kCSPKey, GetDefaultCSPInfo());

  // Always enable security mode when under CSP mode.
  security_mode_enabled_ = true;
  NavigationInfo* info = static_cast<NavigationInfo*>(
      data_->GetManifestData(widget_keys::kAllowNavigationKey));
  if (info) {
    const std::vector<std::string>& allowed_list = info->GetAllowedDomains();
    for (std::vector<std::string>::const_iterator it = allowed_list.begin();
         it != allowed_list.end(); ++it) {
      // If the policy is "*", it represents that any external link is allowed
      // to navigate to.
      if ((*it) == kAsterisk) {
        security_mode_enabled_ = false;
        return;
      }

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
  DCHECK(render_process_host_);
  render_process_host_->Send(
      new ViewMsg_EnableSecurityMode(
          ApplicationData::GetBaseURLFromApplicationId(id()),
          SecurityPolicy::CSP));
}

#if defined(USE_OZONE)
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
