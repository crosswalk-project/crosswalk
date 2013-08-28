// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"

using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionService;

class EchoExtension : public XWalkExtension {
 public:
  EchoExtension() : XWalkExtension() {
    set_name("echo");
  }

  virtual const char* GetJavaScriptAPI() {
    static const char* kAPI =
        "var echoListener = null;"
        "extension.setMessageListener(function(msg) {"
        "  if (echoListener instanceof Function) {"
        "    echoListener(msg);"
        "  };"
        "});"
        "exports.echo = function(msg, callback) {"
        "  echoListener = callback;"
        "  extension.postMessage(msg);"
        "};"
        "exports.syncEcho = function(msg) {"
        "  return extension.internal.sendSyncMessage(msg);"
        "};";
    return kAPI;
  }

  class EchoContext : public XWalkExtension::Context {
   public:
    explicit EchoContext(
        const XWalkExtension::PostMessageCallback& post_message)
        : XWalkExtension::Context(post_message) {}
    virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {
      PostMessage(msg.Pass());
    }
    virtual scoped_ptr<base::Value> HandleSyncMessage(
        scoped_ptr<base::Value> msg) OVERRIDE {
      return msg.Pass();
    }
  };

  virtual Context* CreateContext(
      const XWalkExtension::PostMessageCallback& post_message) {
    return new EchoContext(post_message);
  }
};

class ExtensionWithInvalidName : public XWalkExtension {
 public:
  ExtensionWithInvalidName() : XWalkExtension() {
    set_name("invalid name with spaces");
  }

  virtual const char* GetJavaScriptAPI() { return ""; }
  virtual Context* CreateContext(
      const XWalkExtension::PostMessageCallback& post_message) { return NULL; }
};

class XWalkExtensionsTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionService* extension_service) OVERRIDE {
    ASSERT_TRUE(extension_service->RegisterExtension(new EchoExtension));
    ASSERT_FALSE(
        extension_service->RegisterExtension(new ExtensionWithInvalidName));
  }
};

IN_PROC_BROWSER_TEST_F(XWalkExtensionsTest, EchoExtension) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("test_extension.html"));
  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(XWalkExtensionsTest, EchoExtensionSync) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII(
                                      "sync_echo.html"));
  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
