// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "base/strings/stringprintf.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/spin_wait.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionInstance;
using xwalk::extensions::XWalkExtensionServer;

namespace {

int g_contexts_created = 0;

base::Lock g_contexts_destroyed_lock;
int g_contexts_destroyed = 0;

}

class OnceExtensionInstance : public XWalkExtensionInstance {
 public:
  OnceExtensionInstance()
      : answered_(false) {
  }

  ~OnceExtensionInstance() {
    base::AutoLock lock(g_contexts_destroyed_lock);
    g_contexts_destroyed++;
  }

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {
    std::string answer;
    if (answered_) {
      answer = "Fail";
    } else {
      answer = base::StringPrintf("Pass");
      answered_ = true;
    }
    PostMessageToJS(scoped_ptr<base::Value>(
        base::Value::CreateStringValue(answer)));
  }

 private:
  bool answered_;
};

class OnceExtension : public XWalkExtension {
 public:
  OnceExtension()
      : XWalkExtension() {
    set_name("once");
    set_javascript_api(
        "exports.read = function(callback) {"
        "  extension.setMessageListener(callback);"
        "  extension.postMessage('PING');"
        "};");
  }

  virtual XWalkExtensionInstance* CreateInstance() {
    g_contexts_created++;
    return new OnceExtensionInstance();
  }
};

class XWalkExtensionsContextDestructionTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionServer* server) OVERRIDE {
    ASSERT_TRUE(RegisterExtensionForTest(server, new OnceExtension));
  }

  virtual void TearDown() OVERRIDE {
    SPIN_FOR_1_SECOND_OR_UNTIL_TRUE(g_contexts_destroyed >= 2);
    ASSERT_EQ(g_contexts_destroyed, 2);
  }
};

IN_PROC_BROWSER_TEST_F(XWalkExtensionsContextDestructionTest,
                       ContextIsDestroyedWhenNavigating) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("context_destruction.html"));

  {
    content::TitleWatcher title_watcher(runtime()->web_contents(), kFailString);
    title_watcher.AlsoWaitForTitle(kPassString);
    xwalk_test_utils::NavigateToURL(runtime(), url);
    EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
  }

  {
    content::TitleWatcher title_watcher(runtime()->web_contents(), kFailString);
    title_watcher.AlsoWaitForTitle(kPassString);
    xwalk_test_utils::NavigateToURL(runtime(), url);
    EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
  }

  ASSERT_EQ(g_contexts_created, 2);
}
