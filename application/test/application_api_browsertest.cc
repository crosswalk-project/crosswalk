// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/application/test/application_browsertest.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_registry.h"

// TODO(xiang): add an internal test extension to return API test result
// explicitly.
class ApplicationAPIBrowserTest: public ApplicationBrowserTest {
 public:
  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE;
};

void ApplicationAPIBrowserTest::SetUpCommandLine(CommandLine* command_line) {
  ApplicationBrowserTest::SetUpCommandLine(command_line);
  GURL url = net::FilePathToFileURL(test_data_dir_.Append(
        FILE_PATH_LITERAL("api")));
  command_line->AppendArg(url.spec());
}

IN_PROC_BROWSER_TEST_F(ApplicationAPIBrowserTest, APITest) {
  content::RunAllPendingInMessageLoop();
  const string16 pass = ASCIIToUTF16("Pass");
  const string16 fail = ASCIIToUTF16("Fail");
  // Wait for main document and its opened window ready.
  WaitForRuntimes(2);

  // Wait test result by checking the page title.
  xwalk::Runtime* test_page = xwalk::RuntimeRegistry::Get()->runtimes()[1];
  content::WebContents* web_contents = test_page->web_contents();
  if (web_contents->GetTitle() != pass && web_contents->GetTitle() != fail) {
    content::TitleWatcher title_watcher(web_contents, pass);
    title_watcher.AlsoWaitForTitle(fail);
    EXPECT_EQ(pass, title_watcher.WaitAndGetTitle());
    return;
  }
  ASSERT_EQ(pass, web_contents->GetTitle());
}
