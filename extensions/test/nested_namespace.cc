// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionInstance;
using xwalk::extensions::XWalkExtensionServer;

namespace {

bool g_outer_extension_loaded = false;
bool g_inner_extension_loaded = false;
bool g_another_extension_loaded = false;

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
    set_javascript_api("exports.value = true");
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
    set_javascript_api("exports.value = true;");
  }

  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE {
    return new InnerInstance;
  }
};

class AnotherInstance : public XWalkExtensionInstance {
 public:
  AnotherInstance() {
    g_another_extension_loaded = true;
  }
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {}
};

class AnotherExtension : public XWalkExtension {
 public:
  AnotherExtension() : XWalkExtension() {
    set_name("another");
    // With load on demand enabled, this should cause 'outer' and 'outer.inner'
    // extensions to be loaded. The aim is to guarantee that our trampoline
    // mechanism works in the case that the JS API code depends on another
    // extension.
    set_javascript_api("if (outer.inner.value === true) { "
                       "exports.value = true;"
                       "}");
  }

  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE {
    return new AnotherInstance;
  }
};

class XWalkExtensionsNestedNamespaceTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionServer* server) OVERRIDE {
    ASSERT_TRUE(RegisterExtensionForTest(server, new OuterExtension));
    ASSERT_TRUE(RegisterExtensionForTest(server, new InnerExtension));
  }
};

class XWalkExtensionsTrampolinesForNested : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionServer* server) OVERRIDE {
    ASSERT_TRUE(RegisterExtensionForTest(server, new OuterExtension));
    ASSERT_TRUE(RegisterExtensionForTest(server, new InnerExtension));
    ASSERT_TRUE(RegisterExtensionForTest(server, new AnotherExtension));
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

IN_PROC_BROWSER_TEST_F(XWalkExtensionsNestedNamespaceTest,
                       InstanceNotCreatedForUnusedInnerExtension) {
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

IN_PROC_BROWSER_TEST_F(XWalkExtensionsTrampolinesForNested,
                       InstanceCreatedForExtensionUsedByAnother) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("another.html"));

  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());

  EXPECT_TRUE(g_another_extension_loaded);
  EXPECT_TRUE(g_inner_extension_loaded);
  EXPECT_TRUE(g_outer_extension_loaded);
}
