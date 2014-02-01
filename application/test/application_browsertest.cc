// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/path_service.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/test/application_browsertest.h"
#include "xwalk/application/test/application_testapi.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

using xwalk::application::Application;
using xwalk::application::ApplicationService;
using namespace xwalk::extensions;  // NOLINT

ApplicationBrowserTest::ApplicationBrowserTest()
  : test_runner_(new ApiTestRunner()) {
  PathService::Get(base::DIR_SOURCE_ROOT, &test_data_dir_);
  test_data_dir_ = test_data_dir_
    .Append(FILE_PATH_LITERAL("xwalk"))
    .Append(FILE_PATH_LITERAL("application"))
    .Append(FILE_PATH_LITERAL("test"))
    .Append(FILE_PATH_LITERAL("data"));
}

ApplicationBrowserTest::~ApplicationBrowserTest() {
}

void ApplicationBrowserTest::SetUp() {
  XWalkExtensionService::SetCreateUIThreadExtensionsCallbackForTesting(
      base::Bind(&ApplicationBrowserTest::CreateExtensions,
                 base::Unretained(this)));
  InProcessBrowserTest::SetUp();
}

ApplicationService* ApplicationBrowserTest::application_sevice() const {
  return xwalk::XWalkRunner::GetInstance()->app_system()
      ->application_service();
}

void ApplicationBrowserTest::CreateExtensions(
     XWalkExtensionVector* extensions) {
  ApiTestExtension* extension = new ApiTestExtension;
  extension->SetObserver(test_runner_.get());
  extensions->push_back(extension);
}

IN_PROC_BROWSER_TEST_F(ApplicationBrowserTest, ApiTest) {
  Application* app = application_sevice()->Launch(
      test_data_dir_.Append(FILE_PATH_LITERAL("api")));
  ASSERT_TRUE(app);
  test_runner_->WaitForTestNotification();
  EXPECT_EQ(test_runner_->GetTestsResult(), ApiTestRunner::PASS);
}
