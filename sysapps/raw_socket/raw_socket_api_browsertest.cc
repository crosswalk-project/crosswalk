// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/filename_util.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using namespace xwalk::extensions;  // NOLINT

namespace {

class SysAppsRawSocketTestInstance : public XWalkExtensionInstance {
 public:
  SysAppsRawSocketTestInstance() {}

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {}
};

class SysAppsRawSocketTestExtension : public XWalkExtension {
 public:
  SysAppsRawSocketTestExtension() {
    set_name("sysapps_raw_socket_test");
    set_javascript_api(
        "exports.v8tools = requireNative('v8tools');");
  }

  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE {
    return new SysAppsRawSocketTestInstance();
  }
};

class SysAppsRawSocketTest : public InProcessBrowserTest {
 public:
  virtual void SetUp() {
    XWalkExtensionService::SetCreateUIThreadExtensionsCallbackForTesting(
        base::Bind(&SysAppsRawSocketTest::CreateExtensions,
                   base::Unretained(this)));
    InProcessBrowserTest::SetUp();
  }

  void CreateExtensions(XWalkExtensionVector* extensions) {
    extensions->push_back(new SysAppsRawSocketTestExtension);
  }
};

}  // namespace

IN_PROC_BROWSER_TEST_F(SysAppsRawSocketTest, SysAppsRawSocket) {
  const base::string16 passString = base::ASCIIToUTF16("Pass");
  const base::string16 failString = base::ASCIIToUTF16("Fail");

  content::RunAllPendingInMessageLoop();
  content::TitleWatcher title_watcher(runtime()->web_contents(), passString);
  title_watcher.AlsoWaitForTitle(failString);

  base::FilePath test_file;
  PathService::Get(base::DIR_SOURCE_ROOT, &test_file);
  test_file = test_file
      .Append(FILE_PATH_LITERAL("xwalk"))
      .Append(FILE_PATH_LITERAL("sysapps"))
      .Append(FILE_PATH_LITERAL("raw_socket"))
      .Append(FILE_PATH_LITERAL("raw_socket_api_browsertest.html"));

  xwalk_test_utils::NavigateToURL(runtime(), net::FilePathToFileURL(test_file));
  EXPECT_EQ(passString, title_watcher.WaitAndGetTitle());
}
