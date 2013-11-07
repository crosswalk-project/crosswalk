// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/test/application_apitest.h"

#include "content/public/test/browser_test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/application/test/application_browsertest.h"
#include "xwalk/application/test/application_testapi.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"

using xwalk::extensions::XWalkExtensionService;
using xwalk::extensions::XWalkExtensionServer;

ApplicationApiTest::ApplicationApiTest()
  : test_runner_(new ApiTestRunner()) {
}

ApplicationApiTest::~ApplicationApiTest() {
}

void ApplicationApiTest::SetUp() {
  XWalkExtensionService::SetRegisterUIThreadExtensionsCallbackForTesting(
      base::Bind(&ApplicationApiTest::RegisterExtensions,
                 base::Unretained(this)));
  ApplicationBrowserTest::SetUp();
}

void ApplicationApiTest::SetUpCommandLine(CommandLine* command_line) {
  ApplicationBrowserTest::SetUpCommandLine(command_line);
  GURL url = net::FilePathToFileURL(test_data_dir_.Append(
        FILE_PATH_LITERAL("api")));
  command_line->AppendArg(url.spec());
}

void ApplicationApiTest::RegisterExtensions(XWalkExtensionServer* server) {
  scoped_ptr<ApiTestExtension> extension(new ApiTestExtension);
  extension->SetObserver(test_runner_.get());
  bool registered = server->RegisterExtension(
      extension.PassAs<XWalkExtension>());
  ASSERT_TRUE(registered);
}

IN_PROC_BROWSER_TEST_F(ApplicationApiTest, ApiTest) {
  // Wait for main document and its opened test window ready.
  WaitForRuntimes(2);

  test_runner_->WaitForTestComplete();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::PASS);
}
