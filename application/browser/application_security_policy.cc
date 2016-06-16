// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_security_policy.h"

#include <map>
#include <string>

#include "base/numerics/safe_conversions.h"
#include "content/public/browser/render_process_host.h"
#include "extensions/common/url_pattern.h"
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

ApplicationSecurityPolicy::WhitelistEntry::WhitelistEntry(
    const GURL& dest, const std::string& dest_host, bool subdomains)
    : dest(dest),
      dest_host(dest_host),
      subdomains(subdomains) {
}

bool ApplicationSecurityPolicy::WhitelistEntry::operator==(
    const ApplicationSecurityPolicy::WhitelistEntry& other) const {
  return other.dest == dest &&
         other.dest_host == dest_host &&
         other.subdomains == subdomains;
}

std::unique_ptr<ApplicationSecurityPolicy> ApplicationSecurityPolicy::
    Create(scoped_refptr<ApplicationData> app_data) {
  std::unique_ptr<ApplicationSecurityPolicy> security_policy;
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
    const GURL& policy = entry.dest;
    bool subdomains = entry.subdomains;
    bool is_host_matched = subdomains ?
        url.DomainIs(policy.host().c_str()) : url.host() == policy.host();
    if (url.scheme() == policy.scheme() && is_host_matched &&
        base::StartsWith(url.path(), policy.path(),
                         base::CompareCase::INSENSITIVE_ASCII))
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
        app_url, entry.dest, entry.dest_host, entry.subdomains));
  }

  rph->Send(new ViewMsg_EnableSecurityMode(app_url, mode_));
}

void ApplicationSecurityPolicy::AddWhitelistEntry(
    const GURL& url, const std::string& dest_host, bool subdomains) {
  WhitelistEntry entry = WhitelistEntry(url, dest_host, subdomains);
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
    AddWhitelistEntry(dest_url,
                      dest_url.HostNoBrackets(),
                      (subdomains == "true"));
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
          if (!url.is_valid())
            continue;

          URLPattern allowedUrl(URLPattern::SCHEME_ALL);
          if (allowedUrl.Parse(url.spec()) != URLPattern::PARSE_SUCCESS)
            continue;

          AddWhitelistEntry(url, allowedUrl.host(), allowedUrl.match_subdomains());
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
      AddWhitelistEntry(internalUrl, internalUrl.HostNoBrackets(), false);
    else
      LOG(INFO) << "URL " << internalUrl.spec() << " is wrong.";
  }
}

}  // namespace application
}  // namespace xwalk
