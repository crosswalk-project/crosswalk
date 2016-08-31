// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/test/application_browsertest.h"
#include "xwalk/application/test/application_testapi.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

using xwalk::application::Application;
using xwalk::application::ApplicationService;
using xwalk::application::Manifest;
using xwalk::application::GetManifestPath;

class ApplicationTest : public ApplicationBrowserTest {
};

IN_PROC_BROWSER_TEST_F(ApplicationTest, TestMultiApp) {
  ApplicationService* service = application_sevice();
  const size_t currently_running_count = service->active_applications().size();
  // Launch the first app.
  base::FilePath manifest_path =
      GetManifestPath(test_data_dir_.Append(FILE_PATH_LITERAL("dummy_app1")),
      Manifest::TYPE_MANIFEST);
  Application* app1 = service->LaunchFromManifestPath(
      manifest_path, Manifest::TYPE_MANIFEST);
  ASSERT_TRUE(app1);
  // Wait for app is fully loaded.
  test_runner_->WaitForTestNotification();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::PASS);
  // The App1 has 2 pages: main doc page and "main.html" page.
  EXPECT_EQ(app1->runtimes().size(), 1u);

  EXPECT_EQ(service->active_applications().size(), currently_running_count + 1);
  EXPECT_EQ(service->GetApplicationByID(app1->id()), app1);

  // Verify that no new App instance was created, if one exists
  // with the same ID.
  Application* failed_app1 = service->LaunchFromManifestPath(
      manifest_path, Manifest::TYPE_MANIFEST);
  ASSERT_FALSE(failed_app1);

  // Launch the second app.
  manifest_path =
      GetManifestPath(test_data_dir_.Append(FILE_PATH_LITERAL("dummy_app2")),
          Manifest::TYPE_MANIFEST);
  Application* app2 = application_sevice()->LaunchFromManifestPath(
      manifest_path, Manifest::TYPE_MANIFEST);
  ASSERT_TRUE(app2);
  // Wait for app is fully loaded.
  test_runner_->PostResultToNotificationCallback();
  test_runner_->WaitForTestNotification();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::PASS);

  // The App2 also has 2 pages: main doc page and "main.html" page.
  EXPECT_EQ(app2->runtimes().size(), 1u);

  // Check that the apps have different IDs and RPH IDs.
  EXPECT_NE(app1->id(), app2->id());
  EXPECT_NE(app1->GetRenderProcessHostID(), app2->GetRenderProcessHostID());

  EXPECT_EQ(service->active_applications().size(), currently_running_count + 2);
  EXPECT_EQ(service->GetApplicationByID(app1->id()), app1);
  EXPECT_EQ(service->GetApplicationByID(app2->id()), app2);

  app1->Terminate();
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(service->active_applications().size(), currently_running_count + 1);

  app2->Terminate();
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(service->active_applications().size(), currently_running_count);
}

IN_PROC_BROWSER_TEST_F(ApplicationTest, TestWebStartURL) {
  base::FilePath manifest_path = GetManifestPath(
      test_data_dir_.Append(FILE_PATH_LITERAL("web_start_URL")),
      Manifest::TYPE_MANIFEST);
  Application* app = application_sevice()->LaunchFromManifestPath(
      manifest_path, Manifest::TYPE_MANIFEST);
  ASSERT_TRUE(app);
}
