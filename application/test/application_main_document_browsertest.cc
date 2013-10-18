// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/threading/thread.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_process_manager.h"
#include "xwalk/application/common/application.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/test/application_browsertest.h"
#include "xwalk/runtime/browser/ui/native_app_window.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_registry.h"
#include "xwalk/runtime/common/xwalk_notification_types.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "xwalk/test/base/in_process_browser_test.h"

namespace xwalk {
class NativeAppWindow;
}
namespace {

bool WaitForRuntimeCountCallback(int* count) {
  --(*count);
  return *count == 0;
}

}  // namespace

using base::TimeDelta;
using xwalk::application::Application;
using xwalk::application::ApplicationSystem;
using xwalk::Runtime;
using xwalk::RuntimeRegistry;

class ApplicationMainDocumentBrowserTest: public ApplicationBrowserTest {
 public:
  virtual void SetUp() OVERRIDE;
  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE;

  void CheckMainDocumentStateCb(ApplicationSystem* system,
                                bool suspending,
                                const base::Closure& runner_quit_task);
  void CreateNewRuntime(xwalk::RuntimeContext* context,
                        const GURL& url,
                        const base::Closure& runner_quit_task);

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
  command_line->AppendSwitchASCII(switches::kMainDocumentIdleTime, "1");
  command_line->AppendSwitchASCII(switches::kMainDocumentSuspendingTime, "3");
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

// Verifies close main document.
IN_PROC_BROWSER_TEST_F(ApplicationMainDocumentBrowserTest, CloseMainDocument) {
  content::RunAllPendingInMessageLoop();
  const xwalk::RuntimeList& runtimes = RuntimeRegistry::Get()->runtimes();
  // At least the main document's runtime exist after launch.
  ASSERT_GE(runtimes.size(), 1U);

  Runtime* main_runtime = runtimes[0];
  xwalk::RuntimeContext* runtime_context = main_runtime->runtime_context();
  xwalk::application::ApplicationService* service =
    runtime_context->GetApplicationSystem()->application_service();
  const Application* app = service->GetRunningApplication();
  // Wait for main document and its opened window ready.
  WaitForRuntimes(2);

  xwalk::NativeAppWindow* win1 = runtimes[1]->window();
  ASSERT_TRUE(win1);
  win1->Close();

  // Now only the main document exists, therefore, its state should
  // switch to Suspending after time of |kMainDocumentIdleTime|.
  scoped_refptr<content::MessageLoopRunner> runner =
      new content::MessageLoopRunner;
  base::MessageLoop::current()->PostDelayedTask(
    FROM_HERE,
    base::Bind(&ApplicationMainDocumentBrowserTest::CheckMainDocumentStateCb,
               base::Unretained(this),
               runtime_context->GetApplicationSystem(),
               true,
               runner->QuitClosure()),
    base::TimeDelta::FromSeconds(2));
  runner->Run();
}

// Verifies close main document.
IN_PROC_BROWSER_TEST_F(ApplicationMainDocumentBrowserTest,
                       CancelCloseMainDocument) {
  content::RunAllPendingInMessageLoop();
  const xwalk::RuntimeList& runtimes = RuntimeRegistry::Get()->runtimes();
  // At least the main document's runtime exist after launch.
  ASSERT_GE(runtimes.size(), 1U);

  Runtime* main_runtime = runtimes[0];
  xwalk::RuntimeContext* runtime_context = main_runtime->runtime_context();
  xwalk::application::ApplicationService* service =
    runtime_context->GetApplicationSystem()->application_service();
  const Application* app = service->GetRunningApplication();
  // Wait for main document and its opened window ready.
  WaitForRuntimes(2);

  xwalk::NativeAppWindow* win1 = runtimes[1]->window();
  ASSERT_TRUE(win1);
  win1->Close();

  // Create a new Runtime object when the main document is suspending.
  scoped_refptr<content::MessageLoopRunner> runner =
      new content::MessageLoopRunner;
  GURL url("http://127.0.0.1:2000");
  base::MessageLoop::current()->PostDelayedTask(
    FROM_HERE,
    base::Bind(&ApplicationMainDocumentBrowserTest::CreateNewRuntime,
               base::Unretained(this),
               runtime_context,
               url,
               runner->QuitClosure()),
    base::TimeDelta::FromSeconds(2));
  runner->Run();

  scoped_refptr<content::MessageLoopRunner> runner2 =
      new content::MessageLoopRunner;
  base::MessageLoop::current()->PostDelayedTask(
    FROM_HERE,
    base::Bind(&ApplicationMainDocumentBrowserTest::CheckMainDocumentStateCb,
               base::Unretained(this),
               runtime_context->GetApplicationSystem(),
               false,
               runner2->QuitClosure()),
    base::TimeDelta::FromSeconds(3));
  runner2->Run();
}

void ApplicationMainDocumentBrowserTest::CheckMainDocumentStateCb(
    ApplicationSystem* system,
    bool suspending,
    const base::Closure& runner_quit_task) {
  runner_quit_task.Run();
  ASSERT_EQ(system->process_manager()->IsMainDocumentSuspending(), suspending);
}

void ApplicationMainDocumentBrowserTest::CreateNewRuntime(
    xwalk::RuntimeContext* runtime_context,
    const GURL& url,
    const base::Closure& runner_quit_task) {
  Runtime* new_runtime = Runtime::CreateWithDefaultWindow(
      runtime_context, url);
  runner_quit_task.Run();
}
