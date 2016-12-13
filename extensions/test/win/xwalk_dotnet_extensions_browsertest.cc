// Copyright (c) 2015  Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/native_library.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/test/xwalk_extensions_test_base.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/xwalk_test_utils.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"

using namespace xwalk::extensions;  // NOLINT
using xwalk::extensions::XWalkExtensionService;
using xwalk::Runtime;

class DotNetEchoTest : public XWalkExtensionsTestBase {
 public:
  void SetUp() override {
    XWalkExtensionService::SetExternalExtensionsPathForTesting(
      GetDotNetExtensionTestPath(FILE_PATH_LITERAL("echo_extension")));
    XWalkExtensionsTestBase::SetUp();
  }
};

class DotNetMultipleExtensionTest : public XWalkExtensionsTestBase {
 public:
  void SetUp() override {
    XWalkExtensionService::SetExternalExtensionsPathForTesting(
      GetDotNetExtensionTestPath(FILE_PATH_LITERAL("multiple_extension")));
    XWalkExtensionsTestBase::SetUp();
  }
};

class DotNetBinaryTest : public XWalkExtensionsTestBase {
 public:
  void SetUp() override {
    XWalkExtensionService::SetExternalExtensionsPathForTesting(
      GetDotNetExtensionTestPath(FILE_PATH_LITERAL("binary_extension")));
    XWalkExtensionsTestBase::SetUp();
  }
};

IN_PROC_BROWSER_TEST_F(DotNetEchoTest, DotNetExtension) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(base::FilePath(),
    base::FilePath().AppendASCII("echo.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(DotNetEchoTest, DotNetExtensionSync) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(
    base::FilePath(),
    base::FilePath().AppendASCII("sync_echo.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(DotNetMultipleExtensionTest, DotnetExtensionMultiple) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(
    base::FilePath(),
    base::FilePath().AppendASCII("echo_multiple.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(DotNetBinaryTest, DotNetExtension) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(base::FilePath(),
    base::FilePath().AppendASCII("binaryTest.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
