// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionService;

class TestV8ToolsExtensionContext : public XWalkExtension::Context {
 public:
  explicit TestV8ToolsExtensionContext(
      const XWalkExtension::PostMessageCallback& post_message)
      : XWalkExtension::Context(post_message) {}

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {}
};

class TestV8ToolsExtension : public XWalkExtension {
 public:
  TestV8ToolsExtension()
      : XWalkExtension() {
    set_name("test_v8tools");
  }

  virtual const char* GetJavaScriptAPI() {
    static const char* kAPI =
        "exports.forceSetProperty = function(obj, key, value) {"
        "  var v8tools = requireNative('v8tools');"
        "  v8tools.forceSetProperty(obj, key, value);"
        "};";
    return kAPI;
  }

  virtual Context* CreateContext(
      const XWalkExtension::PostMessageCallback& post_message) {
    return new TestV8ToolsExtensionContext(post_message);
  }
};

class XWalkExtensionsV8ToolsTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionService* extension_service) OVERRIDE {
    bool registered = extension_service->RegisterExtension(
        scoped_ptr<XWalkExtension>(new TestV8ToolsExtension));
    ASSERT_TRUE(registered);
  }
};

IN_PROC_BROWSER_TEST_F(XWalkExtensionsV8ToolsTest,
                       V8ToolsWorks) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("test_v8tools.html"));

  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime(), url);

  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
