// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/test/browser_test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/test/application_browsertest.h"
#include "xwalk/application/test/application_testapi.h"

using xwalk::application::Application;
using xwalk::application::Manifest;
using xwalk::application::GetManifestPath;

class ApplicationTestApiTest : public ApplicationBrowserTest {
};

IN_PROC_BROWSER_TEST_F(ApplicationTestApiTest, TestApiTest) {
  base::FilePath manifest_path =
      GetManifestPath(test_data_dir_.Append(FILE_PATH_LITERAL("testapi")),
      Manifest::TYPE_MANIFEST);
  Application* app = application_sevice()->LaunchFromManifestPath(
      manifest_path, Manifest::TYPE_MANIFEST);
  ASSERT_TRUE(app);
  test_runner_->WaitForTestNotification();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::FAILURE);

  test_runner_->PostResultToNotificationCallback();
  test_runner_->WaitForTestNotification();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::PASS);

  test_runner_->PostResultToNotificationCallback();
  test_runner_->WaitForTestNotification();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::TIMEOUT);

  // xwalk.app.test.runTest will notify pass when all tests are finished.
  test_runner_->PostResultToNotificationCallback();
  test_runner_->WaitForTestNotification();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::PASS);
}
