// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_SECURITY_POLICY_H_
#define XWALK_APPLICATION_COMMON_SECURITY_POLICY_H_
#include "url/gurl.h"

namespace xwalk {
namespace application {

class SecurityPolicy {
 public:
  enum SecurityMode {
    NoSecurity,
    CSP,
    WARP
  };
  SecurityPolicy(const GURL& url, bool subdomains);
  const GURL& url() const { return url_; }
  bool subdomains() const { return subdomains_; }

 private:
  GURL url_;
  bool subdomains_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_SECURITY_POLICY_H_
