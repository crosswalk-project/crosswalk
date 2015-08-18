// Copyright (c) 2013 Intel Corporation. All rights reserved.
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

using xwalk::extensions::XWalkExtensionService;
using xwalk::Runtime;

class ExternalExtensionTest : public XWalkExtensionsTestBase {
 public:
  void SetUp() override {
    XWalkExtensionService::SetExternalExtensionsPathForTesting(
        GetExternalExtensionTestPath(FILE_PATH_LITERAL("echo_extension")));
    XWalkExtensionsTestBase::SetUp();
  }
};

class BulkExtensionTest : public XWalkExtensionsTestBase {
 public:
  void SetUp() override {
    XWalkExtensionService::SetExternalExtensionsPathForTesting(
      GetExternalExtensionTestPath(
        FILE_PATH_LITERAL("bulk_data_transmission")));
    XWalkExtensionsTestBase::SetUp();
  }
};

class RuntimeInterfaceTest : public XWalkExtensionsTestBase {
 public:
  void SetUp() override {
    XWalkExtensionService::SetExternalExtensionsPathForTesting(
        GetExternalExtensionTestPath(
            FILE_PATH_LITERAL("get_runtime_variable")));
    XWalkExtensionsTestBase::SetUp();
  }
};

class MultipleEntryPointsExtension : public XWalkExtensionsTestBase {
 public:
  void SetUp() override {
    XWalkExtensionService::SetExternalExtensionsPathForTesting(
        GetExternalExtensionTestPath(FILE_PATH_LITERAL("multiple_extension")));
    XWalkExtensionsTestBase::SetUp();
  }
};

IN_PROC_BROWSER_TEST_F(ExternalExtensionTest, ExternalExtension) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII("echo.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(ExternalExtensionTest, NavigateWithExternalExtension) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII("echo.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);

  for (int i = 0; i < 5; i++) {
    xwalk_test_utils::NavigateToURL(runtime, url);
    WaitForLoadStop(runtime->web_contents());
    EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
  }
}

IN_PROC_BROWSER_TEST_F(ExternalExtensionTest, ExternalExtensionMessaging2) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(
      base::FilePath(),
      base::FilePath().AppendASCII("echo_messaging_2.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(ExternalExtensionTest, ExternalExtensionSync) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(
      base::FilePath(),
      base::FilePath().AppendASCII("sync_echo.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(RuntimeInterfaceTest, GetRuntimeVariable) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(
      base::FilePath(),
      base::FilePath().AppendASCII("get_runtime_variable.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(MultipleEntryPointsExtension, MultipleEntryPoints) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(
      base::FilePath(),
      base::FilePath().AppendASCII("entry_points.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(MultipleEntryPointsExtension, SetterLoadsExtension) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(
      base::FilePath(),
      base::FilePath().AppendASCII("setter_callback_entry_point.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(MultipleEntryPointsExtension, ReplacementObjectIsUsed) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(
      base::FilePath(),
      base::FilePath().AppendASCII(
          "lazy_loaded_extension_overrides_object.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(BulkExtensionTest, BulkDataExtension) {
Runtime* runtime = CreateRuntime();
GURL url = GetExtensionsTestURL(base::FilePath(),
base::FilePath().AppendASCII("bulk_data_transmission.html"));
content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
title_watcher.AlsoWaitForTitle(kFailString);
xwalk_test_utils::NavigateToURL(runtime, url);
EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
