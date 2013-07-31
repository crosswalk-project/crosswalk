// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_BINDING_BROWSER_BINDING_POLICY_TEST_H_
#define XWALK_BINDING_BROWSER_BINDING_POLICY_TEST_H_

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "xwalk/binding/browser/binding_policy.h"

namespace xwalk {

class BindingPolicyTest : public BindingPolicy::Entry {
 public:
  BindingPolicyTest() {}
  ~BindingPolicyTest() {}
  std::vector<std::string> Grant(const GURL& url) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(BindingPolicyTest);
};

}  // namespace xwalk

#endif  // XWALK_BINDING_BROWSER_BINDING_POLICY_TEST_H_
