// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/base/xwalk_test_utils.h"

#include "base/command_line.h"
#include "base/environment.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/strings/string_number_conversions.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_navigation_observer.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "ui/gl/gl_switches.h"

using content::TestNavigationObserver;
using content::WebContents;

namespace xwalk_test_utils {

void PrepareBrowserCommandLineForTests(CommandLine* command_line) {
  // Enable info level logging by default so that we can see when bad
  // stuff happens, but honor the flags specified from the command line.
  if (!command_line->HasSwitch(switches::kEnableLogging))
    command_line->AppendSwitch(switches::kEnableLogging);
  if (!command_line->HasSwitch(switches::kLoggingLevel))
    command_line->AppendSwitchASCII(switches::kLoggingLevel, "0");

  // Don't collect GPU info, load GPU blacklist, or schedule a GPU blacklist
  // auto-update.
  command_line->AppendSwitch(switches::kSkipGpuDataLoading);
}

bool OverrideDataPathDir(const base::FilePath& data_path_dir) {
  // PathService::Override() is the best way to change the data path directory.
  return PathService::Override(xwalk::DIR_DATA_PATH, data_path_dir);
}

base::FilePath GetTestFilePath(const base::FilePath& dir,
                               const base::FilePath& file) {
  base::FilePath test_base_dir;
  PathService::Get(xwalk::DIR_TEST_DATA, &test_base_dir);

  return test_base_dir.Append(dir).Append(file);
}

GURL GetTestURL(const base::FilePath& dir, const base::FilePath& file) {
  return net::FilePathToFileURL(GetTestFilePath(dir, file));
}

// Navigate a specified URL in the given Runtime. It will block until the
// navigation completes.
void NavigateToURL(xwalk::Runtime* runtime, const GURL& url) {
  if (runtime->web_contents()->IsLoading())
    content::WaitForLoadStop(runtime->web_contents());

  TestNavigationObserver navigation_observer(runtime->web_contents(), 1);
  runtime->LoadURL(url);

  base::RunLoop run_loop;
  navigation_observer.Wait();
}

}  // namespace xwalk_test_utils
