// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using namespace xwalk::extensions;  // NOLINT

class TestExportObjectExtensionInstance : public XWalkExtensionInstance {
 public:
  TestExportObjectExtensionInstance() {}

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {}
};

class TestExportObjectExtension : public XWalkExtension {
 public:
  TestExportObjectExtension()
      : XWalkExtension() {
    set_name("export_object");
    set_javascript_api(
        "exports.data = 54321");
  }

  virtual XWalkExtensionInstance* CreateInstance() {
    return new TestExportObjectExtensionInstance();
  }
};

class TestExportCustomObjectExtension : public XWalkExtension {
 public:
  TestExportCustomObjectExtension()
      : XWalkExtension() {
    set_name("export_custom_object");
    set_javascript_api(
        "var ExportObject = function(data) {"
        "  this.data = data;"
        "};"
        "exports = new ExportObject(12345)");
  }

  virtual XWalkExtensionInstance* CreateInstance() {
    return new TestExportObjectExtensionInstance();
  }
};

class XWalkExtensionsExportObjectTest : public XWalkExtensionsTestBase {
 public:
  virtual void CreateExtensionsForUIThread(
      XWalkExtensionVector* extensions) OVERRIDE {
    extensions->push_back(new TestExportObjectExtension);
    extensions->push_back(new TestExportCustomObjectExtension);
  }
};

IN_PROC_BROWSER_TEST_F(XWalkExtensionsExportObjectTest,
                       ExportObjectWorks) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("test_export_object.html"));

  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime(), url);

  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
