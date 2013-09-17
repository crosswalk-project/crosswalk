// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"
#include "content/public/common/content_switches.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "content/public/browser/web_contents.h"
#include "net/base/net_util.h"
#include "testing/gmock/include/gmock/gmock.h"

using xwalk::Runtime;

class XWalkDevToolsTest : public InProcessBrowserTest {
 public:
  XWalkDevToolsTest() {}
  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE {
    command_line->AppendSwitchASCII(switches::kRemoteDebuggingPort, "9222");
    GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII("test.html"));
    command_line->AppendArg(url.spec());
  }
};

IN_PROC_BROWSER_TEST_F(XWalkDevToolsTest, RemoteDebugging) {
  GURL localhost_url("http://127.0.0.1:9222");
  Runtime* debugging_host = Runtime::CreateWithDefaultWindow(
      runtime()->runtime_context(), localhost_url);
  content::WaitForLoadStop(debugging_host->web_contents());
  string16 real_title = debugging_host->web_contents()->GetTitle();
  string16 expected_title = ASCIIToUTF16("XWalk Remote Debugging");
  EXPECT_EQ(expected_title, real_title);
}
