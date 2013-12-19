// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "url/gurl.h"
#include "xwalk/application/test/application_apitest.h"
#include "xwalk/application/test/application_testapi.h"

class ApplicationEventTest : public ApplicationApiTest {
 public:
  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE {
    ApplicationBrowserTest::SetUpCommandLine(command_line);
    GURL url = net::FilePathToFileURL(test_data_dir_.Append(
          FILE_PATH_LITERAL("event")));
    command_line->AppendArg(url.spec());
  }
};

IN_PROC_BROWSER_TEST_F(ApplicationEventTest, EventTest) {
  test_runner_->WaitForTestNotification();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::PASS);
}
