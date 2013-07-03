// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/extensions/test/cameo_extensions_test_base.h"

#include "cameo/extensions/browser/cameo_extension.h"
#include "cameo/extensions/browser/cameo_extension_service.h"
#include "cameo/runtime/browser/runtime.h"
#include "cameo/test/base/cameo_test_utils.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"

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

class CameoExtensionsTest : public CameoExtensionsTestBase {
 public:
  void RegisterExtensions(CameoExtensionService* extension_service) OVERRIDE {
    extension_service->RegisterExtension(new EchoExtension);
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
