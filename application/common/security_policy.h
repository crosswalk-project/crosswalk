// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_SECURITY_POLICY_H_
#define XWALK_APPLICATION_COMMON_SECURITY_POLICY_H_

#include <vector>

#include "url/gurl.h"

namespace xwalk {
namespace application {

class Application;

// FIXME(Mikhail): Move to application/browser folder.
// Rename to ApplicationSecurityPolicy.
class SecurityPolicy {
 public:
  enum SecurityMode {
    NoSecurity,
    CSP,
    WARP
  };

  explicit SecurityPolicy(Application* app);
  virtual ~SecurityPolicy();

  bool IsAccessAllowed(const GURL& url) const;

  virtual void Enforce() = 0;

 protected:
  struct WhitelistEntry {
    WhitelistEntry(const GURL& url, bool subdomains);
    GURL url;
    bool subdomains;
  };

  void AddWhitelistEntry(const GURL& url, bool subdomains);

  std::vector<WhitelistEntry> whitelist_entries_;
  Application* app_;
  bool enabled_;
};

class SecurityPolicyWARP : public SecurityPolicy {
 public:
  explicit SecurityPolicyWARP(Application* app);
  virtual ~SecurityPolicyWARP();

  virtual void Enforce() OVERRIDE;
};

class SecurityPolicyCSP : public SecurityPolicy {
 public:
  explicit SecurityPolicyCSP(Application* app);
  virtual ~SecurityPolicyCSP();

  virtual void Enforce() OVERRIDE;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_SECURITY_POLICY_H_
