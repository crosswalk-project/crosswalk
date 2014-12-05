// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/native_library.h"
#include "base/path_service.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/extensions/test/xwalk_extensions_test_base.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/xwalk_test_utils.h"
#include "content/public/test/browser_test_utils.h"

using xwalk::extensions::XWalkExtensionService;
using xwalk::Runtime;

class CrashExtensionTest : public XWalkExtensionsTestBase {
 public:
  void SetUp() override {
    XWalkExtensionService::SetExternalExtensionsPathForTesting(
        GetExternalExtensionTestPath(FILE_PATH_LITERAL("crash_extension")));
    XWalkExtensionsTestBase::SetUp();
  }
};

IN_PROC_BROWSER_TEST_F(CrashExtensionTest, CrashExtensionProcessKeepBPAlive) {
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kXWalkDisableExtensionProcess)) {
    LOG(INFO) << "--disable-extension-process not supported by " \
                 "CrashExtensionProcessKeepBPAlive. Skipping test.";
    return;
  }

  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII("crash.html"));
  Runtime* runtime = CreateRuntime();
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);

  xwalk_test_utils::NavigateToURL(runtime, url);
  WaitForLoadStop(runtime->web_contents());

  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
