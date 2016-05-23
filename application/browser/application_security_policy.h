// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SECURITY_POLICY_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SECURITY_POLICY_H_

#include <vector>
#include "base/memory/ref_counted.h"
#include "url/gurl.h"

namespace content {
class RenderProcessHost;
}

namespace xwalk {
namespace application {

class ApplicationData;

class ApplicationSecurityPolicy {
 public:
  enum SecurityMode {
    NoSecurity,
    CSP,
    WARP
  };

  static std::unique_ptr<ApplicationSecurityPolicy> Create(
      scoped_refptr<ApplicationData> app_data);
  virtual ~ApplicationSecurityPolicy();

  bool IsAccessAllowed(const GURL& url) const;

  void EnforceForRenderer(content::RenderProcessHost* rph) const;

 protected:
  struct WhitelistEntry {
    WhitelistEntry(const GURL& dest,
                   const std::string& dest_host,
                   bool subdomains);
    GURL dest;
    std::string dest_host;
    bool subdomains;

    bool operator==(const WhitelistEntry& o) const;
  };

  ApplicationSecurityPolicy(scoped_refptr<ApplicationData> app_data,
                            SecurityMode mode);
  void AddWhitelistEntry(const GURL& url,
                         const std::string& dest_host,
                         bool subdomains);

  virtual void InitEntries() = 0;

  scoped_refptr<ApplicationData> const app_data_;
  std::vector<WhitelistEntry> whitelist_entries_;
  SecurityMode mode_;
  bool enabled_;
};

class ApplicationSecurityPolicyWARP : public ApplicationSecurityPolicy {
 private:
  explicit ApplicationSecurityPolicyWARP(
      scoped_refptr<ApplicationData> app_data);
  ~ApplicationSecurityPolicyWARP() override;

  void InitEntries() override;

  friend class ApplicationSecurityPolicy;
};

class ApplicationSecurityPolicyCSP : public ApplicationSecurityPolicy {
 private:
  explicit ApplicationSecurityPolicyCSP(
      scoped_refptr<ApplicationData> app_data);
  ~ApplicationSecurityPolicyCSP() override;

  void InitEntries() override;

  friend class ApplicationSecurityPolicy;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SECURITY_POLICY_H_
