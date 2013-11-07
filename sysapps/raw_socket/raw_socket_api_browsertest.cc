// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionInstance;
using xwalk::extensions::XWalkExtensionServer;
using xwalk::extensions::XWalkExtensionService;

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
  };

  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE {
    return new SysAppsRawSocketTestInstance();
  }
};

class SysAppsRawSocketTest : public InProcessBrowserTest {
 public:
  void SetUp() {
    XWalkExtensionService::SetRegisterUIThreadExtensionsCallbackForTesting(
        base::Bind(&SysAppsRawSocketTest::RegisterExtensions,
                   base::Unretained(this)));
    InProcessBrowserTest::SetUp();
  }

  void RegisterExtensions(XWalkExtensionServer* server) {
    bool registered = server->RegisterExtension(
        scoped_ptr<XWalkExtension>(new SysAppsRawSocketTestExtension()));
    ASSERT_TRUE(registered);
  }
};

}  // namespace

IN_PROC_BROWSER_TEST_F(SysAppsRawSocketTest, SysAppsRawSocket) {
  const string16 passString = ASCIIToUTF16("Pass");
  const string16 failString = ASCIIToUTF16("Fail");

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
