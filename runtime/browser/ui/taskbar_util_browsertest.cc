// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include "base/command_line.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/runtime/browser/ui/taskbar_util.h"
#include "xwalk/test/base/in_process_browser_test.h"

#if defined(OS_WIN)
#include <shobjidl.h>  // NOLINT(build/include_order)
#endif

class XWalkTaskbarGroupingTest : public InProcessBrowserTest {
 public:
  bool TestForTaskbarGrouping(const std::string& url,
                              const std::string& expected_id) {
    CommandLine::ForCurrentProcess()->AppendArg(url);
    xwalk::SetTaskbarGroupIdForProcess();

    bool result = true;
#if defined(OS_WIN)
    PWSTR user_model_id;
    ::GetCurrentProcessExplicitAppUserModelID(&user_model_id);
    result = (ASCIIToWide(expected_id) == user_model_id);
    CoTaskMemFree(user_model_id);
#endif

    return result;
  }
};

IN_PROC_BROWSER_TEST_F(XWalkTaskbarGroupingTest,
                       UrlWithoutSlashEnd) {
  EXPECT_TRUE(TestForTaskbarGrouping("http://127.0.0.1",
                                     "nffebnloiadhkgeddojeojiaeklddbjj"));
}

IN_PROC_BROWSER_TEST_F(XWalkTaskbarGroupingTest,
                       UrlWithSlashEnd) {
  EXPECT_TRUE(TestForTaskbarGrouping("http://127.0.0.1/",
                                     "nffebnloiadhkgeddojeojiaeklddbjj"));
}

IN_PROC_BROWSER_TEST_F(XWalkTaskbarGroupingTest,
                       WindowsFilePathLowerCaseDriver) {
  EXPECT_TRUE(TestForTaskbarGrouping("c:\\fake-path\\index.html",
                                     "dpamcaeplfebmidmjpbghhhohfkpmnig"));
}

IN_PROC_BROWSER_TEST_F(XWalkTaskbarGroupingTest,
                       WindowsFilePathUpperCaseDriver) {
  EXPECT_TRUE(TestForTaskbarGrouping("C:\\fake-path\\index.html",
                                     "dpamcaeplfebmidmjpbghhhohfkpmnig"));
}

IN_PROC_BROWSER_TEST_F(XWalkTaskbarGroupingTest,
                       FilePathUrl) {
  EXPECT_TRUE(TestForTaskbarGrouping("file:///C:/fake-path/index.html",
                                     "dpamcaeplfebmidmjpbghhhohfkpmnig"));
}
