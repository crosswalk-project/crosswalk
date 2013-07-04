// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/extensions/test/xwalk_extensions_test_base.h"

#include "cameo/extensions/browser/xwalk_extension.h"
#include "cameo/extensions/browser/xwalk_extension_service.h"
#include "cameo/runtime/browser/runtime.h"
#include "cameo/test/base/in_process_browser_test.h"
#include "cameo/test/base/xwalk_test_utils.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"

using cameo::extensions::XWalkExtension;
using cameo::extensions::XWalkExtensionService;

class EchoExtension : public XWalkExtension {
 public:
  EchoExtension() : XWalkExtension() {
    set_name("echo");
  }

  virtual const char* GetJavaScriptAPI() {
    static const char* kAPI =
        "var xwalk = xwalk || {};"
        "xwalk.setMessageListener('echo', function(msg) {"
        "  if (xwalk.echoListener instanceof Function) {"
        "    xwalk.echoListener(msg);"
        "  };"
        "});"
        "xwalk.echo = function(msg, callback) {"
        "  xwalk.echoListener = callback;"
        "  xwalk.postMessage('echo', msg);"
        "};";
    return kAPI;
  }

  class EchoContext : public XWalkExtension::Context {
   public:
    explicit EchoContext(
        const XWalkExtension::PostMessageCallback& post_message)
        : XWalkExtension::Context(post_message) {}
    virtual void HandleMessage(const std::string& msg) OVERRIDE {
      PostMessage(msg);
    }
  };

  virtual Context* CreateContext(
      const XWalkExtension::PostMessageCallback& post_message) {
    return new EchoContext(post_message);
  }
};

class XWalkExtensionsTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionService* extension_service) OVERRIDE {
    extension_service->RegisterExtension(new EchoExtension);
  }
};

IN_PROC_BROWSER_TEST_F(XWalkExtensionsTest, EchoExtension) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII("echo.html"));
  string16 title = ASCIIToUTF16("Pass");
  content::TitleWatcher title_watcher(runtime()->web_contents(), title);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(title, title_watcher.WaitAndGetTitle());
}
