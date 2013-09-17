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
using xwalk::extensions::XWalkExtensionInstance;
using xwalk::extensions::XWalkExtensionService;

class TestV8ToolsExtensionInstance : public XWalkExtensionInstance {
 public:
  explicit TestV8ToolsExtensionInstance() {
  }

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
        "var v8tools = requireNative('v8tools');"
        "exports.forceSetProperty = function(obj, key, value) {"
        "  v8tools.forceSetProperty(obj, key, value);"
        "};"
        "exports.lifecycleTracker = function() {"
        "  return v8tools.lifecycleTracker();"
        "};";
    return kAPI;
  }

  virtual XWalkExtensionInstance* CreateInstance() {
    return new TestV8ToolsExtensionInstance();
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
