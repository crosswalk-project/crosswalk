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

class NamespaceReadOnlyExtensionTest : public XWalkExtensionsTestBase {
 public:
  virtual void SetUp() OVERRIDE {
    XWalkExtensionService::SetExternalExtensionsPathForTesting(
        GetExternalExtensionTestPath(FILE_PATH_LITERAL("multiple_extension")));
    XWalkExtensionsTestBase::SetUp();
  }
};

IN_PROC_BROWSER_TEST_F(NamespaceReadOnlyExtensionTest, NamespaceReadOnly) {
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII(
                                      "namespace_read_only.html"));
  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);

  xwalk_test_utils::NavigateToURL(runtime(), url);
  WaitForLoadStop(runtime()->web_contents());

  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(
    NamespaceReadOnlyExtensionTest, NamespaceReadOnlyAfterEntryPointCalled) {
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("namespace_read_only_with_entrypoint.html"));
  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);

  xwalk_test_utils::NavigateToURL(runtime(), url);
  WaitForLoadStop(runtime()->web_contents());

  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
