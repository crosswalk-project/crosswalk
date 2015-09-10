// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_security_policy.h"

#include <map>
#include <string>

#include "base/numerics/safe_conversions.h"
#include "content/public/browser/render_process_host.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/manifest_handlers/csp_handler.h"
#include "xwalk/application/common/manifest_handlers/tizen_navigation_handler.h"
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

ApplicationSecurityPolicy::WhitelistEntry::WhitelistEntry(
    const GURL& url, bool subdomains)
    : url(url),
      subdomains(subdomains) {
}

ApplicationSecurityPolicy::ApplicationSecurityPolicy(Application* app)
    : app_(app),
      enabled_(false) {
}

ApplicationSecurityPolicy::~ApplicationSecurityPolicy() {
}

bool ApplicationSecurityPolicy::IsAccessAllowed(const GURL& url) const {
  if (!enabled_)
    return true;

  // Accessing own resources in W3C Widget is always allowed.
  if (app_->data()->manifest_type() == Manifest::TYPE_WIDGET &&
      url.SchemeIs(application::kApplicationScheme) && url.host() == app_->id())
    return true;

  for (std::vector<WhitelistEntry>::const_iterator it =
      whitelist_entries_.begin(); it != whitelist_entries_.end(); ++it) {
    const GURL& policy = it->url;
    bool subdomains = it->subdomains;
    bool is_host_matched = subdomains ?
        url.DomainIs(policy.host().c_str()) : url.host() == policy.host();
    if (url.scheme() == policy.scheme() && is_host_matched &&
        base::StartsWithASCII(url.path(), policy.path(), false))
      return true;
  }
  return false;
}

void ApplicationSecurityPolicy::Enforce() {
}

void ApplicationSecurityPolicy::AddWhitelistEntry(
    const GURL& url, bool subdomains) {
  GURL app_url = app_->data()->URL();
  DCHECK(app_->render_process_host());
  WhitelistEntry entry = WhitelistEntry(url, subdomains);

  std::vector<WhitelistEntry>::iterator it =
      std::find(whitelist_entries_.begin(), whitelist_entries_.end(), entry);
  if (it != whitelist_entries_.end())
    return;

  app_->render_process_host()->Send(new ViewMsg_SetAccessWhiteList(
      app_url, url, subdomains));
  whitelist_entries_.push_back(entry);
}

ApplicationSecurityPolicyWARP::ApplicationSecurityPolicyWARP(Application* app)
    : ApplicationSecurityPolicy(app) {
}

ApplicationSecurityPolicyWARP::~ApplicationSecurityPolicyWARP() {
}

void ApplicationSecurityPolicyWARP::Enforce() {
  const WARPInfo* info = static_cast<WARPInfo*>(
      app_->data()->GetManifestData(widget_keys::kAccessKey));
  if (!info) {
    enabled_ = true;
    DCHECK(app_->render_process_host());
    app_->render_process_host()->Send(
        new ViewMsg_EnableSecurityMode(
            ApplicationData::GetBaseURLFromApplicationId(app_->id()),
            ApplicationSecurityPolicy::WARP));
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
            ApplicationSecurityPolicy::WARP));
  }
}

ApplicationSecurityPolicyCSP::ApplicationSecurityPolicyCSP(Application* app)
    : ApplicationSecurityPolicy(app) {
}

ApplicationSecurityPolicyCSP::~ApplicationSecurityPolicyCSP() {
}

void ApplicationSecurityPolicyCSP::Enforce() {
  Manifest::Type manifest_type = app_->data()->manifest_type();
  const char* scp_key = GetCSPKey(manifest_type);
  CSPInfo* csp_info =
      static_cast<CSPInfo*>(app_->data()->GetManifestData(scp_key));
  if (manifest_type == Manifest::TYPE_WIDGET) {
#if defined(OS_TIZEN)
    if (!csp_info || csp_info->GetDirectives().empty())
       app_->data()->SetManifestData(scp_key, GetDefaultCSPInfo());
    // Always enable security mode when under CSP mode.
    enabled_ = true;
    TizenNavigationInfo* info = static_cast<TizenNavigationInfo*>(
        app_->data()->GetManifestData(widget_keys::kAllowNavigationKey));
    if (info) {
      const std::vector<std::string>& allowed_list = info->GetAllowedDomains();
      for (const auto& it : allowed_list) {
        // If the policy is "*", it represents that any external link is allowed
        // to navigate to.
        if (it == kAsterisk) {
          enabled_ = false;
          return;
        }

        // If the policy start with "*.", like this: *.domain,
        // means that can access to all subdomains for 'domain',
        // otherwise, the host of request url should exactly the same
        // as policy.
        bool subdomains = (it.find("*.") == 0);
        const std::string host = subdomains ? it.substr(2) : it;
        AddWhitelistEntry(GURL("http://" + host), subdomains);
        AddWhitelistEntry(GURL("https://" + host), subdomains);
      }
    }
#endif
  } else {
    if (csp_info && !csp_info->GetDirectives().empty()) {
      enabled_ = true;
      const std::map<std::string, std::vector<std::string> >& policies =
          csp_info->GetDirectives();

      for (const auto& directive : policies) {
        for (const auto& it : directive.second) {
          GURL url(it);
          if (url.is_valid())
            AddWhitelistEntry(url, false);
        }
      }
    }

    // scope
    std::string scope;
    GURL internalUrl(ApplicationData::GetBaseURLFromApplicationId(app_->id()));
    if (app_->data()->GetManifest()->GetString(keys::kScopeKey, &scope) &&
        !scope.empty()) {
      enabled_ = true;
      url::Replacements<char> replacements;
      replacements.SetPath(
                   scope.c_str(),
                   url::Component(0, base::checked_cast<int>(scope.length())));
      internalUrl = internalUrl.ReplaceComponents(replacements);
    }
    if (internalUrl.is_valid())
      // All links out of whitelist will be opened in system default web
      // browser.
      AddWhitelistEntry(internalUrl, false);
    else
      LOG(INFO) << "URL " << internalUrl.spec() << " is wrong.";
  }

  if (enabled_) {
    DCHECK(app_->render_process_host());
    app_->render_process_host()->Send(
        new ViewMsg_EnableSecurityMode(
            ApplicationData::GetBaseURLFromApplicationId(app_->id()),
            ApplicationSecurityPolicy::CSP));
  }
}

}  // namespace application
}  // namespace xwalk
