// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "base/stringprintf.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/spin_wait.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionService;

namespace {

int g_contexts_created = 0;

base::Lock g_contexts_destroyed_lock;
int g_contexts_destroyed = 0;

}

class OnceExtensionContext : public XWalkExtension::Context {
 public:
  OnceExtensionContext(int sequence,
      const XWalkExtension::PostMessageCallback& post_message)
      : XWalkExtension::Context(post_message),
        sequence_(sequence),
        answered_(false) {}

  ~OnceExtensionContext() {
    base::AutoLock lock(g_contexts_destroyed_lock);
    g_contexts_destroyed++;
  }

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {
    std::string answer;
    if (answered_) {
      answer = "FAIL";
    } else {
      answer = base::StringPrintf("PASS %d", sequence_);
      answered_ = true;
    }
    PostMessage(scoped_ptr<base::Value>(
        base::Value::CreateStringValue(answer)));
  }

 private:
  int sequence_;
  bool answered_;
};

class OnceExtension : public XWalkExtension {
 public:
  OnceExtension()
      : XWalkExtension() {
    set_name("once");
  }

  virtual const char* GetJavaScriptAPI() {
    static const char* kAPI =
        "exports.read = function(callback) {"
        "  extension.setMessageListener(callback);"
        "  extension.postMessage('PING');"
        "};";
    return kAPI;
  }

  virtual Context* CreateContext(
      const XWalkExtension::PostMessageCallback& post_message) {
    return new OnceExtensionContext(++g_contexts_created, post_message);
  }
};

class XWalkExtensionsContextDestructionTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionService* extension_service) OVERRIDE {
    bool registered = extension_service->RegisterExtension(
        scoped_ptr<XWalkExtension>(new OnceExtension));
    ASSERT_TRUE(registered);
  }

  virtual void TearDown() OVERRIDE {
    SPIN_FOR_1_SECOND_OR_UNTIL_TRUE(g_contexts_destroyed == 2);
    ASSERT_EQ(g_contexts_destroyed, 2);
  }
};

IN_PROC_BROWSER_TEST_F(XWalkExtensionsContextDestructionTest,
                       ContextIsDestroyedWhenNavigating) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("context_destruction.html"));

  string16 fail = ASCIIToUTF16("FAIL");

  {
    content::TitleWatcher title_watcher(runtime()->web_contents(), fail);
    string16 pass_1 = ASCIIToUTF16("PASS 1");
    title_watcher.AlsoWaitForTitle(pass_1);
    xwalk_test_utils::NavigateToURL(runtime(), url);
    EXPECT_EQ(pass_1, title_watcher.WaitAndGetTitle());
  }

  {
    content::TitleWatcher title_watcher(runtime()->web_contents(), fail);
    string16 pass_2 = ASCIIToUTF16("PASS 2");
    title_watcher.AlsoWaitForTitle(pass_2);
    xwalk_test_utils::NavigateToURL(runtime(), url);
    EXPECT_EQ(pass_2, title_watcher.WaitAndGetTitle());
  }

  ASSERT_EQ(g_contexts_created, 2);
}
