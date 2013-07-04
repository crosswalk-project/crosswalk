// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/utf_string_conversions.h"
#include "cameo/runtime/browser/runtime.h"
#include "cameo/test/base/in_process_browser_test.h"
#include "cameo/test/base/xwalk_test_utils.h"
#include "content/public/common/content_switches.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "content/public/browser/web_contents.h"
#include "net/base/net_util.h"
#include "testing/gmock/include/gmock/gmock.h"

using cameo::Runtime;

class CameoDevToolsTest : public InProcessBrowserTest {
 public:
  CameoDevToolsTest() {}
  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE {
    command_line->AppendSwitchASCII(switches::kRemoteDebuggingPort, "9222");
    GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII("test.html"));
    command_line->AppendArg(url.spec());
  }
};

IN_PROC_BROWSER_TEST_F(CameoDevToolsTest, RemoteDebugging) {
  GURL localhost_url("http://127.0.0.1:9222");
  Runtime* debugging_host =
      Runtime::Create(runtime()->runtime_context(), localhost_url);
  content::WaitForLoadStop(debugging_host->web_contents());
  string16 real_title = debugging_host->web_contents()->GetTitle();
  string16 expected_title = ASCIIToUTF16("Cameo Remote Debugging");
  EXPECT_EQ(expected_title, real_title);
}
