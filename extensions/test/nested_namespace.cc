// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionInstance;
using xwalk::extensions::XWalkExtensionService;

namespace {

bool g_outer_extension_loaded = false;
bool g_inner_extension_loaded = false;

}

class OuterInstance : public XWalkExtensionInstance {
 public:
  OuterInstance() {
    g_outer_extension_loaded = true;
  }
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {}
};

class OuterExtension : public XWalkExtension {
 public:
  OuterExtension() : XWalkExtension() {
    set_name("outer");
  }

  virtual const char* GetJavaScriptAPI() {
    static const char* kAPI =
        "exports.value = true";
    return kAPI;
  }

  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE {
    return new OuterInstance;
  }
};

class InnerInstance : public XWalkExtensionInstance {
 public:
  InnerInstance() {
    g_inner_extension_loaded = true;
  }
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {}
};

class InnerExtension : public XWalkExtension {
 public:
  InnerExtension() : XWalkExtension() {
    set_name("outer.inner");
  }

  virtual const char* GetJavaScriptAPI() {
    static const char* kAPI =
        "exports.value = true";
    return kAPI;
  }

  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE {
    return new InnerInstance;
  }
};

class XWalkExtensionsNestedNamespaceTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionService* extension_service) OVERRIDE {
    bool registered_outer = extension_service->RegisterExtension(
        scoped_ptr<XWalkExtension>(new OuterExtension));
    ASSERT_TRUE(registered_outer);
    bool registered_inner = extension_service->RegisterExtension(
        scoped_ptr<XWalkExtension>(new InnerExtension));
    ASSERT_TRUE(registered_inner);
  }
};

IN_PROC_BROWSER_TEST_F(XWalkExtensionsNestedNamespaceTest,
                       InstanceCreatedForInnerExtension) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("inner_outer.html"));

  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());

  EXPECT_TRUE(g_outer_extension_loaded);
  EXPECT_TRUE(g_inner_extension_loaded);
}

// TODO(cmarcelo): This will be enabled when we fully land load on demand.
IN_PROC_BROWSER_TEST_F(XWalkExtensionsNestedNamespaceTest,
                       DISABLED_InstanceNotCreatedForUnusedInnerExtension) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("outer.html"));

  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());

  EXPECT_TRUE(g_outer_extension_loaded);
  EXPECT_FALSE(g_inner_extension_loaded);
}
