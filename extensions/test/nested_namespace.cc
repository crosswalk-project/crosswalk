// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using namespace xwalk::extensions;  // NOLINT
using xwalk::Runtime;

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
  void HandleMessage(scoped_ptr<base::Value> msg) override {}
};

class OuterExtension : public XWalkExtension {
 public:
  OuterExtension() : XWalkExtension() {
    set_name("outer");
    set_javascript_api("exports.value = true");
  }

  XWalkExtensionInstance* CreateInstance() override {
    return new OuterInstance;
  }
};

class InnerInstance : public XWalkExtensionInstance {
 public:
  InnerInstance() {
    g_inner_extension_loaded = true;
  }
  void HandleMessage(scoped_ptr<base::Value> msg) override {}
};

class InnerExtension : public XWalkExtension {
 public:
  InnerExtension() : XWalkExtension() {
    set_name("outer.inner");
    set_javascript_api("exports.value = true;");
  }

  XWalkExtensionInstance* CreateInstance() override {
    return new InnerInstance;
  }
};

class AnotherInstance : public XWalkExtensionInstance {
 public:
  AnotherInstance() {
    g_another_extension_loaded = true;
  }
  void HandleMessage(scoped_ptr<base::Value> msg) override {}
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

  XWalkExtensionInstance* CreateInstance() override {
    return new AnotherInstance;
  }
};

class XWalkExtensionsNestedNamespaceTest : public XWalkExtensionsTestBase {
 public:
  void CreateExtensionsForUIThread(
      XWalkExtensionVector* extensions) override {
    extensions->push_back(new OuterExtension);
    extensions->push_back(new InnerExtension);
  }
};

class XWalkExtensionsTrampolinesForNested : public XWalkExtensionsTestBase {
 public:
  void CreateExtensionsForUIThread(
      XWalkExtensionVector* extensions) override {
    extensions->push_back(new OuterExtension);
    extensions->push_back(new InnerExtension);
    extensions->push_back(new AnotherExtension);
  }
};

IN_PROC_BROWSER_TEST_F(XWalkExtensionsNestedNamespaceTest,
                       InstanceCreatedForInnerExtension) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("inner_outer.html"));

  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());

  EXPECT_TRUE(g_outer_extension_loaded);
  EXPECT_TRUE(g_inner_extension_loaded);
}

IN_PROC_BROWSER_TEST_F(XWalkExtensionsNestedNamespaceTest,
                       InstanceNotCreatedForUnusedInnerExtension) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("outer.html"));

  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());

  EXPECT_TRUE(g_outer_extension_loaded);
  EXPECT_FALSE(g_inner_extension_loaded);
}

IN_PROC_BROWSER_TEST_F(XWalkExtensionsTrampolinesForNested,
                       InstanceCreatedForExtensionUsedByAnother) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("another.html"));

  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());

  EXPECT_TRUE(g_another_extension_loaded);
  EXPECT_TRUE(g_inner_extension_loaded);
  EXPECT_TRUE(g_outer_extension_loaded);
}
