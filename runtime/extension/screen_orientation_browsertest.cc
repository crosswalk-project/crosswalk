// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file_path.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"

const string16 kPassString = ASCIIToUTF16("Pass");
const string16 kFailString = ASCIIToUTF16("Fail");

class XWalkScreenOrientationTest : public InProcessBrowserTest {
};

IN_PROC_BROWSER_TEST_F(XWalkScreenOrientationTest, APITest) {
  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath().AppendASCII("orientation"),
      base::FilePath().AppendASCII("test.html"));
  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
