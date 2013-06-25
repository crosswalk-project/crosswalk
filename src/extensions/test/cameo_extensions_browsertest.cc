// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/path_service.h"
#include "base/utf_string_conversions.h"
#include "cameo/src/extensions/browser/cameo_extension.h"
#include "cameo/src/extensions/browser/cameo_extension_service.h"
#include "cameo/src/runtime/browser/runtime.h"
#include "cameo/src/test/base/cameo_test_utils.h"
#include "cameo/src/test/base/in_process_browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"

using cameo::extensions::CameoExtension;
using cameo::extensions::CameoExtensionService;

class EchoExtension : public CameoExtension {
 public:
  EchoExtension() : CameoExtension("echo") {}

  virtual const char* GetJavaScriptAPI() {
    static const char* kAPI =
        "var cameo = cameo || {};"
        "cameo.setMessageListener('echo', function(msg) {"
        "  if (cameo.echoListener instanceof Function) {"
        "    cameo.echoListener(msg);"
        "  };"
        "});"
        "cameo.echo = function(msg, callback) {"
        "  cameo.echoListener = callback;"
        "  cameo.postMessage('echo', msg);"
        "};";
    return kAPI;
  }

  class EchoContext : public CameoExtension::Context {
   public:
    explicit EchoContext(
        const CameoExtension::PostMessageCallback& post_message)
        : CameoExtension::Context(post_message) {}
    virtual void HandleMessage(const std::string& msg) OVERRIDE {
      PostMessage(msg);
    }
  };

  virtual Context* CreateContext(
      const CameoExtension::PostMessageCallback& post_message) {
    return new EchoContext(post_message);
  }
};

class CameoExtensionsTest : public InProcessBrowserTest {
 public:
  virtual void SetUp() OVERRIDE {
    CameoExtensionService::SetRegisterExtensionsCallbackForTesting(
        base::Bind(&CameoExtensionsTest::RegisterExtensions,
                   base::Unretained(this)));
    InProcessBrowserTest::SetUp();
  }

  void RegisterExtensions(CameoExtensionService* extension_service) {
    extension_service->RegisterExtension(new EchoExtension);
  }

  GURL GetExtensionsTestURL(const base::FilePath& dir,
                            const base::FilePath& file) {
    base::FilePath test_file;
    PathService::Get(base::DIR_SOURCE_ROOT, &test_file);
    test_file = test_file
                .Append(FILE_PATH_LITERAL("cameo"))
                .Append(FILE_PATH_LITERAL("src"))
                .Append(FILE_PATH_LITERAL("extensions"))
                .Append(FILE_PATH_LITERAL("test"))
                .Append(FILE_PATH_LITERAL("data"))
                .Append(dir).Append(file);
    return net::FilePathToFileURL(test_file);
  }
};

IN_PROC_BROWSER_TEST_F(CameoExtensionsTest, EchoExtension) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII("echo.html"));
  string16 title = ASCIIToUTF16("Pass");
  content::TitleWatcher title_watcher(runtime()->web_contents(), title);
  cameo_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(title, title_watcher.WaitAndGetTitle());
}
