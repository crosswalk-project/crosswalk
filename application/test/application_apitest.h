// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TEST_APPLICATION_APITEST_H_
#define XWALK_APPLICATION_TEST_APPLICATION_APITEST_H_

#include "xwalk/application/test/application_browsertest.h"

class ApiTestRunner;

namespace xwalk {
namespace extensions {
class XWalkExtensionServer;
class XWalkExtensionService;
}
}

class ApplicationApiTest : public ApplicationBrowserTest {
 public:
  ApplicationApiTest();
  virtual ~ApplicationApiTest();

 protected:
  virtual void SetUp() OVERRIDE;
  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE;

  scoped_ptr<ApiTestRunner> test_runner_;

 private:
  void RegisterExtensions(xwalk::extensions::XWalkExtensionServer* server);
};

#endif  // XWALK_APPLICATION_TEST_APPLICATION_APITEST_H_
