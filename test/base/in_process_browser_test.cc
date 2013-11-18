// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/base/in_process_browser_test.h"

#include "base/auto_reset.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/debug/leak_annotations.h"
#include "base/files/file_path.h"
#include "base/file_util.h"
#include "base/lazy_instance.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/test/test_file_util.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "content/public/common/content_switches.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_browser_thread.h"
#include "content/public/test/test_launcher.h"
#include "content/public/test/test_navigation_observer.h"
#include "content/public/test/test_utils.h"
#include "net/test/spawned_test_server/spawned_test_server.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_registry.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "xwalk/runtime/renderer/xwalk_content_renderer_client.h"
#include "xwalk/test/base/xwalk_test_suite.h"
#include "xwalk/test/base/xwalk_test_utils.h"

#if defined(OS_TIZEN_MOBILE)
#include "xwalk/runtime/renderer/tizen/xwalk_content_renderer_client_tizen.h"
#endif

using xwalk::RuntimeList;
using xwalk::RuntimeRegistry;
using xwalk::XWalkContentRendererClient;

namespace {

// Used when running in single-process mode.
#if defined(OS_TIZEN_MOBILE)
base::LazyInstance<XWalkContentRendererClientTizen>::Leaky
        g_xwalk_content_renderer_client = LAZY_INSTANCE_INITIALIZER;
#else
base::LazyInstance<XWalkContentRendererClient>::Leaky
        g_xwalk_content_renderer_client = LAZY_INSTANCE_INITIALIZER;
#endif

}  // namespace

InProcessBrowserTest::InProcessBrowserTest()
    : runtime_(NULL) {
  CreateTestServer(base::FilePath(FILE_PATH_LITERAL("xwalk/test/data")));
}

InProcessBrowserTest::~InProcessBrowserTest() {
}

CommandLine InProcessBrowserTest::GetCommandLineForRelaunch() {
  CommandLine new_command_line(CommandLine::ForCurrentProcess()->GetProgram());
  CommandLine::SwitchMap switches =
      CommandLine::ForCurrentProcess()->GetSwitches();
  new_command_line.AppendSwitch(content::kLaunchAsBrowser);

  for (CommandLine::SwitchMap::const_iterator iter = switches.begin();
        iter != switches.end(); ++iter) {
    new_command_line.AppendSwitchNative((*iter).first, (*iter).second);
  }
  return new_command_line;
}

void InProcessBrowserTest::SetUp() {
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  // Allow subclasses to change the command line before running any tests.
  SetUpCommandLine(command_line);
  // Add command line arguments that are used by all InProcessBrowserTests.
  PrepareTestCommandLine(command_line);

  // Single-process mode is not set in BrowserMain, so process it explicitly,
  // and set up renderer.
  if (command_line->HasSwitch(switches::kSingleProcess)) {
    content::SetRendererClientForTesting(
        &g_xwalk_content_renderer_client.Get());
  }

  BrowserTestBase::SetUp();
}

void InProcessBrowserTest::PrepareTestCommandLine(CommandLine* command_line) {
  // Propagate commandline settings from test_launcher_utils.
  xwalk_test_utils::PrepareBrowserCommandLineForTests(command_line);
}

void InProcessBrowserTest::TearDown() {
  BrowserTestBase::TearDown();
}

void InProcessBrowserTest::RunTestOnMainThreadLoop() {
  // Pump startup related events.
  content::RunAllPendingInMessageLoop();

  const RuntimeList& runtimes = RuntimeRegistry::Get()->runtimes();
  if (!runtimes.empty()) {
    runtime_ = runtimes.at(0);
      content::WaitForLoadStop(runtime_->web_contents());
  }

  content::RunAllPendingInMessageLoop();

  SetUpOnMainThread();

  if (!HasFatalFailure())
    RunTestOnMainThread();

  // Invoke cleanup and quit even if there are failures. This is similar to
  // gtest in that it invokes TearDown even if Setup fails.
  CleanUpOnMainThread();
  // Sometimes tests leave Quit tasks in the MessageLoop (for shame), so let's
  // run all pending messages here to avoid preempting the QuitBrowsers tasks.
  content::RunAllPendingInMessageLoop();

  QuitAllRuntimes();
}

bool InProcessBrowserTest::CreateDataPathDir() {
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  base::FilePath data_path_dir =
      command_line->GetSwitchValuePath(switches::kXWalkDataPath);
  if (data_path_dir.empty()) {
    if (temp_data_path_dir_.CreateUniqueTempDir() &&
        temp_data_path_dir_.IsValid()) {
      data_path_dir = temp_data_path_dir_.path();
    } else {
      LOG(ERROR) << "Could not create temporary data directory \""
                 << temp_data_path_dir_.path().value() << "\".";
      return false;
    }
  }
  return xwalk_test_utils::OverrideDataPathDir(data_path_dir);
}

void InProcessBrowserTest::QuitAllRuntimes() {
  RuntimeRegistry::Get()->CloseAll();
}
