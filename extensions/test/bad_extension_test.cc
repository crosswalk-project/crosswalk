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
using xwalk::extensions::XWalkExtensionServer;

class BadExtensionTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionService* extension_service,
      XWalkExtensionServer* server) OVERRIDE {
    base::FilePath extension_dir;
    PathService::Get(base::DIR_EXE, &extension_dir);

    extension_dir = extension_dir
                    .Append(FILE_PATH_LITERAL("tests"))
                    .Append(FILE_PATH_LITERAL("extension"))
                    .Append(FILE_PATH_LITERAL("bad_extension"));

    extension_service->RegisterExternalExtensionsForPath(extension_dir);
  }
};

IN_PROC_BROWSER_TEST_F(BadExtensionTest, DoNotCrash) {
  content::RunAllPendingInMessageLoop();
  LOG(WARNING) << "This test will produce a lot warnings which are expected."
               << " The goal is to not crash.";
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII("bad.html"));
  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(BadExtensionTest, NavigateDoNotCrash) {
  content::RunAllPendingInMessageLoop();
  LOG(WARNING) << "This test will produce a lot warnings which are expected."
               << " The goal is to not crash.";
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII("bad.html"));
  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);

  for (int i = 0; i < 5; i++) {
    xwalk_test_utils::NavigateToURL(runtime(), url);
    WaitForLoadStop(runtime()->web_contents());
    EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
  }
}
