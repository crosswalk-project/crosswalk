// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "base/task_runner.h"
#include "base/time/time.h"

using namespace xwalk::extensions;  // NOLINT
using xwalk::Runtime;

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
  void HandleMessage(scoped_ptr<base::Value> msg) override {
    PostMessageToJS(msg.Pass());
  }
  void HandleSyncMessage(scoped_ptr<base::Value> msg) override {
    SendSyncReplyToJS(msg.Pass());
  }
};

class DelayedEchoContext : public XWalkExtensionInstance {
 public:
  void HandleMessage(scoped_ptr<base::Value> msg) override {
    PostMessageToJS(msg.Pass());
  }
  void HandleSyncMessage(scoped_ptr<base::Value> msg) override {
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

  XWalkExtensionInstance* CreateInstance() override {
    s_instance_was_created = true;
    return new EchoContext();
  }

  static bool s_instance_was_created;
};

bool EchoExtension::s_instance_was_created = false;

class DelayedEchoExtension : public XWalkExtension {
 public:
  DelayedEchoExtension() : XWalkExtension() {
    set_name("echo");
    set_javascript_api(kEchoAPI);
  }

  XWalkExtensionInstance* CreateInstance() override {
    return new DelayedEchoContext();
  }
};

class ExtensionWithInvalidName : public XWalkExtension {
 public:
  ExtensionWithInvalidName() : XWalkExtension() {
    set_name("invalid name with spaces");
  }

  XWalkExtensionInstance* CreateInstance() override {
    s_instance_was_created = true;
    return NULL;
  }

  static bool s_instance_was_created;
};

bool ExtensionWithInvalidName::s_instance_was_created = false;

static const char* kBulkDataAPI = "var bulkDataListener = null;"
""
"extension.setMessageListener(function(msg) {"
"  if (bulkDataListener instanceof Function) {"
"    bulkDataListener(msg);"
"  };"
"});"
""
"exports.requestBulkDataAsync = function(power, callback) {"
"  bulkDataListener = callback;"
"  extension.postMessage(power.toString());"
"};";

class BulkDataContext : public XWalkExtensionInstance {
 public:
  BulkDataContext() {
  }
  void HandleMessage(scoped_ptr<base::Value> msg) override {
    std::string message;
    msg->GetAsString(&message);
    int size = atoi(message.c_str());
    std::string data_chunk(size, 'p');
    scoped_ptr<base::Value> data(new base::StringValue(data_chunk));
    PostMessageToJS(data.Pass());
  }
};

class BulkDataExtension : public XWalkExtension {
 public:
  BulkDataExtension() : XWalkExtension() {
    set_name("bulkData");
    set_javascript_api(kBulkDataAPI);
  }

  XWalkExtensionInstance* CreateInstance() override {
    return new BulkDataContext();
  }
};

}  // namespace

class XWalkExtensionsTest : public XWalkExtensionsTestBase {
 public:
  void CreateExtensionsForUIThread(
      XWalkExtensionVector* extensions) override {
    extensions->push_back(new EchoExtension);
    extensions->push_back(new ExtensionWithInvalidName);
    extensions->push_back(new BulkDataExtension);
  }
};

class XWalkExtensionsDelayedTest : public XWalkExtensionsTestBase {
 public:
  void CreateExtensionsForUIThread(
      XWalkExtensionVector* extensions) override {
    extensions->push_back(new DelayedEchoExtension);
  }
};

IN_PROC_BROWSER_TEST_F(XWalkExtensionsTest, EchoExtension) {
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("test_extension.html"));
  Runtime* runtime = CreateRuntime();
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(XWalkExtensionsTest, ExtensionWithInvalidNameIgnored) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("test_extension.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());

  EXPECT_TRUE(EchoExtension::s_instance_was_created);
  EXPECT_FALSE(ExtensionWithInvalidName::s_instance_was_created);
}

IN_PROC_BROWSER_TEST_F(XWalkExtensionsTest, EchoExtensionSync) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII(
                                      "sync_echo.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(XWalkExtensionsDelayedTest, EchoExtensionSync) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII(
                                      "sync_echo.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(XWalkExtensionsTest, BulkDataExtension) {
  Runtime* runtime = CreateRuntime();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("bulk_data_transmission.html"));
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);
  xwalk_test_utils::NavigateToURL(runtime, url);
  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
