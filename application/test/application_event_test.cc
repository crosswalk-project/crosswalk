// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/file_util.h"
#include "base/path_service.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/filename_util.h"
#include "url/gurl.h"
#include "xwalk/application/test/application_browsertest.h"
#include "xwalk/application/test/application_testapi.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#include "xwalk/runtime/common/xwalk_switches.h"

using xwalk::application::Application;

class ApplicationEventTest : public ApplicationBrowserTest {
 public:
  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE {
    ApplicationBrowserTest::SetUpCommandLine(command_line);
    GURL url = net::FilePathToFileURL(test_data_dir_.Append(
          FILE_PATH_LITERAL("event")));
    command_line->AppendArg(url.spec());
  }
};

class ApplicationOnInstalledEventTest : public ApplicationBrowserTest {
 public:
  ApplicationOnInstalledEventTest() {
    PathService::Get(xwalk::DIR_TEST_DATA, &app_data_path_);
    app_data_path_ = app_data_path_
        .Append(FILE_PATH_LITERAL("appdata_storage"));
  }

  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE {
    DeleteApplicationDataPath();

    ApplicationBrowserTest::SetUpCommandLine(command_line);
    command_line->AppendSwitchPath(switches::kXWalkDataPath, app_data_path_);
    command_line->AppendSwitch(switches::kInstall);

    GURL url = net::FilePathToFileURL(test_data_dir_.Append(
          FILE_PATH_LITERAL("event")));
    command_line->AppendArg(url.spec());
  }

  bool DeleteApplicationDataPath() {
    base::FilePath resources(app_data_path_);
    if (base::DirectoryExists(resources) &&
        !base::DeleteFile(resources, true))
      return false;
    return true;
  }

 private:
  base::FilePath app_data_path_;
};

IN_PROC_BROWSER_TEST_F(ApplicationEventTest, OnLaunchAndSuspendEventTest) {
  // Wait for 'OnLaunch' event notification.
  test_runner_->WaitForTestNotification();
  EXPECT_EQ(ApiTestRunner::PASS, test_runner_->GetTestsResult());

  EXPECT_EQ(application_sevice()->active_applications().size(), 1);
  Application* app =
      application_sevice()->active_applications()[0];
  test_runner_->PostResultToNotificationCallback();
  app->Terminate();
  // Wait for 'OnSuspend' event notification.
  test_runner_->WaitForTestNotification();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::PASS);
}

IN_PROC_BROWSER_TEST_F(ApplicationOnInstalledEventTest, OnInstalledEventTest) {
  test_runner_->WaitForTestNotification();
  EXPECT_EQ(ApiTestRunner::PASS, test_runner_->GetTestsResult());

  DeleteApplicationDataPath();
}
