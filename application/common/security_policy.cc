// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/security_policy.h"

namespace xwalk {
namespace application {

SecurityPolicy::SecurityPolicy(const GURL& url, bool subdomains)
    : url_(url),
      subdomains_(subdomains) {
}

}  // namespace application
}  // namespace xwalk
