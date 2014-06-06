// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/security_policy.h"

#include <string>

#include "content/public/browser/render_process_host.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/manifest_handlers/csp_handler.h"
#include "xwalk/application/common/manifest_handlers/navigation_handler.h"
#include "xwalk/application/common/manifest_handlers/warp_handler.h"
#include "xwalk/runtime/common/xwalk_common_messages.h"

namespace xwalk {

namespace keys = application_manifest_keys;
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

// By default:
// default-src * ; script-src 'self' ; style-src 'self' ; object-src 'none'
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

SecurityPolicy::WhitelistEntry::WhitelistEntry(const GURL& url, bool subdomains)
  : url(url),
    subdomains(subdomains) {
}

SecurityPolicy::SecurityPolicy(Application* app)
  : app_(app),
    enabled_(false) {
}

SecurityPolicy::~SecurityPolicy() {
}

bool SecurityPolicy::IsAccessAllowed(const GURL& url) const {
  if (!enabled_)
    return true;

  // Accessing own resources is always allowed.
  if (url.SchemeIs(application::kApplicationScheme) &&
      url.host() == app_->id())
    return true;

  for (std::vector<WhitelistEntry>::const_iterator it =
      whitelist_entries_.begin(); it != whitelist_entries_.end(); ++it) {
    const GURL& policy = it->url;
    bool subdomains = it->subdomains;
    bool is_host_matched = subdomains ?
        url.DomainIs(policy.host().c_str()) : url.host() == policy.host();
    if (url.scheme() == policy.scheme() && is_host_matched)
      return true;
  }
  return false;
}

void SecurityPolicy::Enforce() {
}

void SecurityPolicy::AddWhitelistEntry(const GURL& url, bool subdomains) {
  GURL app_url = app_->data()->URL();
  DCHECK(app_->render_process_host());
  app_->render_process_host()->Send(
      new ViewMsg_SetAccessWhiteList(
          app_url, url, subdomains));
  whitelist_entries_.push_back(WhitelistEntry(url, subdomains));
}

SecurityPolicyWARP::SecurityPolicyWARP(Application* app)
  : SecurityPolicy(app) {
}

SecurityPolicyWARP::~SecurityPolicyWARP() {
}

void SecurityPolicyWARP::Enforce() {
  const WARPInfo* info = static_cast<WARPInfo*>(
      app_->data()->GetManifestData(widget_keys::kAccessKey));
  if (!info) {
    enabled_ = true;
    DCHECK(app_->render_process_host());
    app_->render_process_host()->Send(
        new ViewMsg_EnableSecurityMode(
            ApplicationData::GetBaseURLFromApplicationId(app_->id()),
            SecurityPolicy::WARP));
    return;
  }

  const base::ListValue* whitelist = info->GetWARP();
  for (base::ListValue::const_iterator it = whitelist->begin();
       it != whitelist->end(); ++it) {
    base::DictionaryValue* value = NULL;
    (*it)->GetAsDictionary(&value);
    std::string dest;
    if (!value || !value->GetString(widget_keys::kAccessOriginKey, &dest) ||
        dest.empty())
      continue;
    if (dest == "*") {
      enabled_ = false;
      break;
    }

    GURL dest_url(dest);
    // The default subdomains attribute should be "false".
    std::string subdomains = "false";
    value->GetString(widget_keys::kAccessSubdomainsKey, &subdomains);
    AddWhitelistEntry(dest_url, (subdomains == "true"));
    enabled_ = true;
  }

  if (enabled_) {
    DCHECK(app_->render_process_host());
    app_->render_process_host()->Send(
        new ViewMsg_EnableSecurityMode(
            ApplicationData::GetBaseURLFromApplicationId(app_->id()),
            SecurityPolicy::WARP));
  }
}

SecurityPolicyCSP::SecurityPolicyCSP(Application* app)
  : SecurityPolicy(app) {
}

SecurityPolicyCSP::~SecurityPolicyCSP() {
}

void SecurityPolicyCSP::Enforce() {
#if defined(OS_TIZEN)
  Package::Type package_type = app_->data()->GetPackageType();
  const char* scp_key = GetCSPKey(package_type);

  CSPInfo* csp_info =
      static_cast<CSPInfo*>(app_->data()->GetManifestData(scp_key));
  if (!csp_info || csp_info->GetDirectives().empty())
    app_->data()->SetManifestData(scp_key, GetDefaultCSPInfo());

  // Always enable security mode when under CSP mode.
  enabled_ = true;

  if (package_type = Package::WGT) {
    NavigationInfo* info = static_cast<NavigationInfo*>(
        app_->data()->GetManifestData(widget_keys::kAllowNavigationKey));
    if (info) {
      const std::vector<std::string>& allowed_list = info->GetAllowedDomains();
      for (std::vector<std::string>::const_iterator it = allowed_list.begin();
           it != allowed_list.end(); ++it) {
        // If the policy is "*", it represents that any external link is allowed
        // to navigate to.
        if ((*it) == kAsterisk) {
          enabled_ = false;
          return;
        }

        // If the policy start with "*.", like this: *.domain,
        // means that can access to all subdomains for 'domain',
        // otherwise, the host of request url should exactly the same
        // as policy.
        bool subdomains = ((*it).find("*.") == 0);
        std::string host = subdomains ? (*it).substr(2) : (*it);
        AddWhitelistEntry(GURL("http://" + host), subdomains);
        AddWhitelistEntry(GURL("https://" + host), subdomains);
      }
    }
  } else {
    // FIXME: Implement http://w3c.github.io/manifest-csp/ here.
  }

  DCHECK(app_->render_process_host());
  app_->render_process_host()->Send(
      new ViewMsg_EnableSecurityMode(
          ApplicationData::GetBaseURLFromApplicationId(app_->id()),
          SecurityPolicy::CSP));
#endif
}

}  // namespace application
}  // namespace xwalk
