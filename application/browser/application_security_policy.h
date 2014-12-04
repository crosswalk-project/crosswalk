// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SECURITY_POLICY_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SECURITY_POLICY_H_

#include <vector>

#include "url/gurl.h"

namespace xwalk {
namespace application {

class Application;

class ApplicationSecurityPolicy {
 public:
  enum SecurityMode {
    NoSecurity,
    CSP,
    WARP
  };

  explicit ApplicationSecurityPolicy(Application* app);
  virtual ~ApplicationSecurityPolicy();

  bool IsAccessAllowed(const GURL& url) const;

  virtual void Enforce() = 0;

 protected:
  struct WhitelistEntry {
    WhitelistEntry(const GURL& url, bool subdomains);
    GURL url;
    bool subdomains;

    bool operator==(const WhitelistEntry& o) const {
      return o.url == url && o.subdomains == subdomains;
    }
  };

  void AddWhitelistEntry(const GURL& url, bool subdomains);

  std::vector<WhitelistEntry> whitelist_entries_;
  Application* app_;
  bool enabled_;
};

class ApplicationSecurityPolicyWARP : public ApplicationSecurityPolicy {
 public:
  explicit ApplicationSecurityPolicyWARP(Application* app);
  virtual ~ApplicationSecurityPolicyWARP();

  void Enforce() override;
};

class ApplicationSecurityPolicyCSP : public ApplicationSecurityPolicy {
 public:
  explicit ApplicationSecurityPolicyCSP(Application* app);
  virtual ~ApplicationSecurityPolicyCSP();

  void Enforce() override;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SECURITY_POLICY_H_
