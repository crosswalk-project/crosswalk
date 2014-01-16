// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/test/application_browsertest.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

using xwalk::application::Application;

class ApplicationMainDocumentBrowserTest: public ApplicationBrowserTest {
};

// Verifies the runtime creation when main document is used.
IN_PROC_BROWSER_TEST_F(ApplicationMainDocumentBrowserTest, MainDocument) {
  content::RunAllPendingInMessageLoop();
  // At least the main document's runtime exist after launch.
  ASSERT_GE(GetRuntimeCount(), 1);

  xwalk::application::ApplicationService* service =
      xwalk::XWalkRunner::GetInstance()->app_system()->application_service();
  Application* app = service->Launch(
      test_data_dir_.Append(FILE_PATH_LITERAL("main_document")));
  ASSERT_TRUE(app);
  // There should exist 2 runtimes(one for generated main document, one for the
  // window created by main document).
  WaitForRuntimes(2);

  GURL generated_url =
      app->data()->GetResourceURL(
          xwalk::application::kGeneratedMainDocumentFilename);
  xwalk::Runtime* main_runtime = app->GetMainDocumentRuntime();
  ASSERT_TRUE(main_runtime);
  // Check main document URL.
  ASSERT_EQ(main_runtime->web_contents()->GetURL(), generated_url);
  ASSERT_TRUE(!main_runtime->window());
}
