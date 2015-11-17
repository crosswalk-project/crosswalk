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
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/manifest_handlers/csp_handler.h"
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

scoped_ptr<ApplicationSecurityPolicy> ApplicationSecurityPolicy::
    Create(scoped_refptr<ApplicationData> app_data) {
  scoped_ptr<ApplicationSecurityPolicy> security_policy;
  // CSP policy takes precedence over WARP.
  if (app_data->HasCSPDefined())
    security_policy.reset(new ApplicationSecurityPolicyCSP(app_data));
  else if (app_data->manifest_type() == Manifest::TYPE_WIDGET)
    security_policy.reset(new ApplicationSecurityPolicyWARP(app_data));

  if (security_policy)
    security_policy->InitEntries();

  return security_policy;
}

ApplicationSecurityPolicy::ApplicationSecurityPolicy(
    scoped_refptr<ApplicationData> app_data, SecurityMode mode)
    : app_data_(app_data),
      mode_(mode),
      enabled_(false) {
}

ApplicationSecurityPolicy::~ApplicationSecurityPolicy() {
}

bool ApplicationSecurityPolicy::IsAccessAllowed(const GURL& url) const {
  if (!enabled_)
    return true;

  // Accessing own resources in W3C Widget is always allowed.
  if (app_data_->manifest_type() == Manifest::TYPE_WIDGET &&
      url.SchemeIs(kApplicationScheme) && url.host() == app_data_->ID())
    return true;

  for (const WhitelistEntry& entry : whitelist_entries_) {
    const GURL& policy = entry.url;
    bool subdomains = entry.subdomains;
    bool is_host_matched = subdomains ?
        url.DomainIs(policy.host().c_str()) : url.host() == policy.host();
    if (url.scheme() == policy.scheme() && is_host_matched &&
        base::StartsWithASCII(url.path(), policy.path(), false))
      return true;
  }
  return false;
}

void ApplicationSecurityPolicy::EnforceForRenderer(
    content::RenderProcessHost* rph) const {
  DCHECK(rph);

  if (!enabled_)
    return;

  DCHECK(!whitelist_entries_.empty());
  const GURL& app_url = app_data_->URL();
  for (const WhitelistEntry& entry : whitelist_entries_) {
    rph->Send(new ViewMsg_SetAccessWhiteList(
        app_url, entry.url, entry.subdomains));
  }

  rph->Send(new ViewMsg_EnableSecurityMode(app_url, mode_));
}

void ApplicationSecurityPolicy::AddWhitelistEntry(
    const GURL& url, bool subdomains) {
  WhitelistEntry entry = WhitelistEntry(url, subdomains);
  auto it =
      std::find(whitelist_entries_.begin(), whitelist_entries_.end(), entry);
  if (it != whitelist_entries_.end())
    return;

  whitelist_entries_.push_back(entry);
}

ApplicationSecurityPolicyWARP::ApplicationSecurityPolicyWARP(
    scoped_refptr<ApplicationData> app_data)
    : ApplicationSecurityPolicy(app_data, ApplicationSecurityPolicy::WARP) {
}

ApplicationSecurityPolicyWARP::~ApplicationSecurityPolicyWARP() {
}

void ApplicationSecurityPolicyWARP::InitEntries() {
  const WARPInfo* info = static_cast<WARPInfo*>(
      app_data_->GetManifestData(widget_keys::kAccessKey));
  if (!info) {
    enabled_ = true;
    return;
  }

  const base::ListValue* whitelist = info->GetWARP();
  for (base::ListValue::const_iterator it = whitelist->begin();
       it != whitelist->end(); ++it) {
    base::DictionaryValue* value = nullptr;
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
}

ApplicationSecurityPolicyCSP::ApplicationSecurityPolicyCSP(
    scoped_refptr<ApplicationData> app_data)
    : ApplicationSecurityPolicy(app_data, ApplicationSecurityPolicy::CSP) {
}

ApplicationSecurityPolicyCSP::~ApplicationSecurityPolicyCSP() {
}

void ApplicationSecurityPolicyCSP::InitEntries() {
  Manifest::Type manifest_type = app_data_->manifest_type();
  const char* scp_key = GetCSPKey(manifest_type);
  CSPInfo* csp_info = static_cast<CSPInfo*>(
      app_data_->GetManifestData(scp_key));
  if (manifest_type != Manifest::TYPE_WIDGET) {
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
    GURL internalUrl = app_data_->URL();
    if (app_data_->GetManifest()->GetString(keys::kScopeKey, &scope) &&
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
}

}  // namespace application
}  // namespace xwalk
