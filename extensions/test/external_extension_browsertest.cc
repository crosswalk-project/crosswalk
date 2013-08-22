// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/native_library.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension_external.h"
#include "xwalk/extensions/test/xwalk_extensions_test_base.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/xwalk_test_utils.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"

using xwalk::extensions::XWalkExtensionService;
using xwalk::extensions::old::XWalkExternalExtension;

static base::FilePath GetNativeLibraryFilePath(const char* name) {
  base::string16 library_name = base::GetNativeLibraryName(UTF8ToUTF16(name));
#if defined(OS_WIN)
  return base::FilePath(library_name);
#else
  return base::FilePath(UTF16ToUTF8(library_name));
#endif
}

class OldExternalExtensionTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionService* extension_service) OVERRIDE {
    base::FilePath extension_file;
    PathService::Get(base::DIR_EXE, &extension_file);
    extension_file = extension_file.Append(
        GetNativeLibraryFilePath("external_extension_sample"));
    XWalkExternalExtension* extension =
        new XWalkExternalExtension(extension_file);
    ASSERT_TRUE(extension->is_valid());
    extension_service->RegisterExtension(extension);
  }
};

IN_PROC_BROWSER_TEST_F(OldExternalExtensionTest, ExternalExtension) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII("echo.html"));
  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}


IN_PROC_BROWSER_TEST_F(OldExternalExtensionTest, ExternalExtensionSync) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(
      base::FilePath(),
      base::FilePath().AppendASCII("sync_echo.html"));
  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
