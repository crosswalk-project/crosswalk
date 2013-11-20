// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/application.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/test/application_browsertest.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/runtime_registry.h"

using xwalk::application::Application;

class ApplicationMainDocumentBrowserTest: public ApplicationBrowserTest {
 public:
  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE;
};

void ApplicationMainDocumentBrowserTest::SetUpCommandLine(
    CommandLine* command_line) {
  ApplicationBrowserTest::SetUpCommandLine(command_line);
  GURL url = net::FilePathToFileURL(test_data_dir_.Append(
        FILE_PATH_LITERAL("main_document")));
  command_line->AppendArg(url.spec());
}

// Verifies the runtime creation when main document is used.
IN_PROC_BROWSER_TEST_F(ApplicationMainDocumentBrowserTest, MainDocument) {
  content::RunAllPendingInMessageLoop();
  // At least the main document's runtime exist after launch.
  ASSERT_GE(GetRuntimeNumber(), 1);

  xwalk::Runtime* main_runtime = xwalk::RuntimeRegistry::Get()->runtimes()[0];
  xwalk::RuntimeContext* runtime_context = main_runtime->runtime_context();
  xwalk::application::ApplicationService* service =
    runtime_context->GetApplicationSystem()->application_service();
  const Application* app = service->GetRunningApplication();
  GURL generated_url =
    app->GetResourceURL(xwalk::application::kGeneratedMainDocumentFilename);
  // Check main document URL.
  ASSERT_EQ(main_runtime->web_contents()->GetURL(), generated_url);
  ASSERT_TRUE(!main_runtime->window());

  // There should exist 2 runtimes(one for generated main document, one for the
  // window created by main document).
  WaitForRuntimes(2);
}
