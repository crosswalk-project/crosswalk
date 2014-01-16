// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TEST_APPLICATION_APITEST_H_
#define XWALK_APPLICATION_TEST_APPLICATION_APITEST_H_

#include <vector>
#include "xwalk/application/test/application_browsertest.h"
#include "xwalk/extensions/common/xwalk_extension_vector.h"

class ApiTestRunner;

class ApplicationApiTest : public ApplicationBrowserTest {
 public:
  ApplicationApiTest();
  virtual ~ApplicationApiTest();

 protected:
  virtual void SetUp() OVERRIDE;

  scoped_ptr<ApiTestRunner> test_runner_;

 private:
  void CreateExtensions(xwalk::extensions::XWalkExtensionVector* extensions);
};

#endif  // XWALK_APPLICATION_TEST_APPLICATION_APITEST_H_
