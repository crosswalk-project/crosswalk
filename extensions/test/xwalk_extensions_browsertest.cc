// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "base/task_runner.h"
#include "base/time/time.h"

using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionInstance;
using xwalk::extensions::XWalkExtensionServer;

namespace {

const char* kEchoAPI =
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

class EchoContext : public XWalkExtensionInstance {
 public:
  EchoContext() {
  }
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {
    PostMessageToJS(msg.Pass());
  }
  virtual void HandleSyncMessage(scoped_ptr<base::Value> msg) OVERRIDE {
    SendSyncReplyToJS(msg.Pass());
  }
};

class DelayedEchoContext : public XWalkExtensionInstance {
 public:
  explicit DelayedEchoContext() {
  }
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {
    PostMessageToJS(msg.Pass());
  }
  virtual void HandleSyncMessage(scoped_ptr<base::Value> msg) OVERRIDE {
    base::MessageLoop::current()->PostDelayedTask(
        FROM_HERE, base::Bind(&DelayedEchoContext::DelayedReply,
                              base::Unretained(this), base::Passed(&msg)),
        base::TimeDelta::FromSeconds(1));
  }

  void DelayedReply(scoped_ptr<base::Value> reply) {
    SendSyncReplyToJS(reply.Pass());
  }
};

class EchoExtension : public XWalkExtension {
 public:
  EchoExtension() : XWalkExtension() {
    set_name("echo");
    set_javascript_api(kEchoAPI);
  }

  virtual XWalkExtensionInstance* CreateInstance() {
    return new EchoContext();
  }
};

class DelayedEchoExtension : public XWalkExtension {
 public:
  DelayedEchoExtension() : XWalkExtension() {
    set_name("echo");
    set_javascript_api(kEchoAPI);
  }

  virtual XWalkExtensionInstance* CreateInstance() {
    return new DelayedEchoContext();
  }
};

class ExtensionWithInvalidName : public XWalkExtension {
 public:
  ExtensionWithInvalidName() : XWalkExtension() {
    set_name("invalid name with spaces");
  }

  virtual XWalkExtensionInstance* CreateInstance() { return NULL; }
};

}  // namespace

class XWalkExtensionsTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionServer* server) OVERRIDE {
    ASSERT_TRUE(RegisterExtensionForTest(server, new EchoExtension));
    ASSERT_FALSE(RegisterExtensionForTest(
        server, new ExtensionWithInvalidName));
  }
};

class XWalkExtensionsDelayedTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionServer* server) OVERRIDE {
    ASSERT_TRUE(RegisterExtensionForTest(server, new DelayedEchoExtension));
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

IN_PROC_BROWSER_TEST_F(XWalkExtensionsDelayedTest, EchoExtensionSync) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII(
                                      "sync_echo.html"));
  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
