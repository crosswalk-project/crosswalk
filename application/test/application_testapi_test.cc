// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/test/browser_test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/application/test/application_apitest.h"
#include "xwalk/application/test/application_testapi.h"

class ApplicationTestApiTest : public ApplicationApiTest {
 public:
  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE;
};

void ApplicationTestApiTest::SetUpCommandLine(CommandLine* command_line) {
  ApplicationBrowserTest::SetUpCommandLine(command_line);
  GURL url = net::FilePathToFileURL(test_data_dir_.Append(
        FILE_PATH_LITERAL("testapi")));
  command_line->AppendArg(url.spec());
}

IN_PROC_BROWSER_TEST_F(ApplicationTestApiTest, TestApiTest) {
  // Wait for main document open.
  WaitForRuntimes(1);

  test_runner_->WaitForTestComplete();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::FAILURE);

  // We must reset the result before wait again.
  test_runner_->ResetResult();
  test_runner_->WaitForTestComplete();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::PASS);
}
