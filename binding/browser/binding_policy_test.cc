// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/binding/browser/binding_policy_test.h"

#include "url/gurl.h"

namespace xwalk {

std::vector<std::string> BindingPolicyTest::Grant(const GURL& url) {
  std::vector<std::string> features;
  if (url.scheme() == "file" || url.has_query()) {
    std::string query = url.query();
    size_t p0 = 0;
    do {
      size_t p = query.find('&', p0);
      if (p == std::string::npos)
        p = query.length();
      features.push_back(query.substr(p0, p - p0));
      p0 = p + 1;
    }while (p0 < query.size());
  }
  return features;
}

}  // namespace xwalk
