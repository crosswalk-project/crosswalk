// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "base/values.h"
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

bool g_clean_extension_loaded = false;
bool g_dirty_extension_loaded = false;

base::ListValue g_clean_entry_points;
base::ListValue g_conflicts_with_name;
base::ListValue g_conflicts_with_entry_points;

}

// We should be prepared against extensions conflicting with each others
// entry points, either by the extension name or by explicit entry points.

class CleanInstance : public XWalkExtensionInstance {
 public:
  CleanInstance() {
    g_clean_extension_loaded = true;
  }
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {}
};

class DirtyInstance : public XWalkExtensionInstance {
 public:
  DirtyInstance() {
    g_dirty_extension_loaded = true;
  }
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {}
};

class CleanExtension : public XWalkExtension {
 public:
  CleanExtension() : XWalkExtension() {
    set_name("clean");
    set_entry_points(std::vector<std::string>(1, std::string("FromClean")));
    set_javascript_api("exports.clean_loaded = true;"
                       "window.FromClean = true;");
  }

  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE {
    return new CleanInstance;
  }
};

class ConflictsWithNameExtension : public XWalkExtension {
 public:
  ConflictsWithNameExtension() : XWalkExtension() {
    set_name("conflicts_with_name");
    set_entry_points(std::vector<std::string>(1, std::string("clean")));
    set_javascript_api("window.clean = 'fail';");
  }

  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE {
    return new DirtyInstance;
  }
};


class ConflictsWithEntryPointExtension
    : public XWalkExtension {
 public:
  ConflictsWithEntryPointExtension() : XWalkExtension() {
    set_name("conflicts_with_entry_point");
    set_entry_points(std::vector<std::string>(1, std::string("FromClean")));
    set_javascript_api("window.FromClean = 'fail';");
  }

  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE {
    return new DirtyInstance;
  }
};

class XWalkExtensionsConflictsWithNameTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionServer* server) OVERRIDE {
    ASSERT_TRUE(RegisterExtensionForTest(server, new CleanExtension));
    ASSERT_FALSE(RegisterExtensionForTest(
        server, new ConflictsWithNameExtension));
  }
};

class XWalkExtensionsConflictsWithEntryPointTest
    : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionServer* server) OVERRIDE {
    ASSERT_TRUE(RegisterExtensionForTest(server, new CleanExtension));
    ASSERT_FALSE(RegisterExtensionForTest(
        server, new ConflictsWithEntryPointExtension));
  }
};

IN_PROC_BROWSER_TEST_F(XWalkExtensionsConflictsWithNameTest,
                       OnlyCleanInstanceLoaded) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(
      base::FilePath(),
      base::FilePath().AppendASCII("conflicting_names.html"));

  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());

  EXPECT_TRUE(g_clean_extension_loaded);
  EXPECT_FALSE(g_dirty_extension_loaded);
}

IN_PROC_BROWSER_TEST_F(XWalkExtensionsConflictsWithEntryPointTest,
                       OnlyCleanInstanceLoaded) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(
      base::FilePath(),
      base::FilePath().AppendASCII("conflicting_names.html"));

  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());

  EXPECT_TRUE(g_clean_extension_loaded);
  EXPECT_FALSE(g_dirty_extension_loaded);
}
