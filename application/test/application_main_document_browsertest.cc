// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/application.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_registry.h"
#include "xwalk/runtime/common/xwalk_notification_types.h"
#include "xwalk/test/base/in_process_browser_test.h"

namespace {

bool WaitForRuntimeCountCallback(int* count) {
  --(*count);
  return *count == 0;
}

}  // namespace

using xwalk::application::Application;
using xwalk::Runtime;
using xwalk::RuntimeRegistry;

class ApplicationMainDocumentBrowserTest: public InProcessBrowserTest {
 public:
  virtual void SetUp() OVERRIDE;
  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE;

  base::FilePath test_data_dir_;
};

void ApplicationMainDocumentBrowserTest::SetUp() {
  PathService::Get(base::DIR_SOURCE_ROOT, &test_data_dir_);
  test_data_dir_ = test_data_dir_
    .Append(FILE_PATH_LITERAL("xwalk"))
    .Append(FILE_PATH_LITERAL("application"))
    .Append(FILE_PATH_LITERAL("test"))
    .Append(FILE_PATH_LITERAL("data"));
  InProcessBrowserTest::SetUp();
}

void ApplicationMainDocumentBrowserTest::SetUpCommandLine(
    CommandLine* command_line) {
  GURL url = net::FilePathToFileURL(test_data_dir_.Append(
        FILE_PATH_LITERAL("main_document")));
  command_line->AppendArg(url.spec());
}

// Verifies the runtime creation when main document is used.
IN_PROC_BROWSER_TEST_F(ApplicationMainDocumentBrowserTest, MainDocument) {
  content::RunAllPendingInMessageLoop();
  const xwalk::RuntimeList& runtimes = RuntimeRegistry::Get()->runtimes();
  // At least the main document's runtime exist after launch.
  ASSERT_GE(runtimes.size(), 1U);

  Runtime* main_runtime = runtimes[0];
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
  int count = 2 - runtimes.size();
  if (count) {
    content::WindowedNotificationObserver(
        xwalk::NOTIFICATION_RUNTIME_OPENED,
        base::Bind(&WaitForRuntimeCountCallback, &count)).Wait();
  }

  ASSERT_EQ(2, RuntimeRegistry::Get()->runtimes().size());
}
