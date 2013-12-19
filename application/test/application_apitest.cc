// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/test/application_apitest.h"

#include <vector>
#include "content/public/test/browser_test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/application/test/application_browsertest.h"
#include "xwalk/application/test/application_testapi.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"

using namespace xwalk::extensions;  // NOLINT

ApplicationApiTest::ApplicationApiTest()
  : test_runner_(new ApiTestRunner()) {
}

ApplicationApiTest::~ApplicationApiTest() {
}

void ApplicationApiTest::SetUp() {
  XWalkExtensionService::SetCreateUIThreadExtensionsCallbackForTesting(
      base::Bind(&ApplicationApiTest::CreateExtensions,
                 base::Unretained(this)));
  ApplicationBrowserTest::SetUp();
}

void ApplicationApiTest::SetUpCommandLine(CommandLine* command_line) {
  ApplicationBrowserTest::SetUpCommandLine(command_line);
  GURL url = net::FilePathToFileURL(test_data_dir_.Append(
        FILE_PATH_LITERAL("api")));
  command_line->AppendArg(url.spec());
}

void ApplicationApiTest::CreateExtensions(XWalkExtensionVector* extensions) {
  ApiTestExtension* extension = new ApiTestExtension;
  extension->SetObserver(test_runner_.get());
  extensions->push_back(extension);
}

IN_PROC_BROWSER_TEST_F(ApplicationApiTest, ApiTest) {
  test_runner_->WaitForTestNotification();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::PASS);
}
