// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/browser/browser_thread.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"
#include "xwalk/extensions/test/xwalk_extensions_test_base.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionInstance;
using xwalk::extensions::XWalkExtensionServer;
using xwalk::extensions::XWalkExtensionService;

const char kInProcessExtensionThread[] = "in_process_extension_thread";
const char kInProcessUIThread[] = "in_process_ui_thread";

class InProcessExtension;

class InProcessExtensionInstance : public XWalkExtensionInstance {
 public:
  InProcessExtensionInstance() {}

  scoped_ptr<base::Value> InRunningOnUIThread() {
    bool is_on_ui_thread =
        content::BrowserThread::CurrentlyOn(content::BrowserThread::UI);

    scoped_ptr<base::ListValue> reply(new base::ListValue);
    reply->AppendBoolean(is_on_ui_thread);

    return reply.PassAs<base::Value>();
  }

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {
    PostMessageToJS(InRunningOnUIThread());
  }

  virtual void HandleSyncMessage(scoped_ptr<base::Value> msg) OVERRIDE {
    SendSyncReplyToJS(InRunningOnUIThread());
  }
};

class InProcessExtension : public XWalkExtension {
 public:
  explicit InProcessExtension(const char* name) {
    set_name(name);
    set_javascript_api(
      "var listener = null;"
      "extension.setMessageListener(function(msg) {"
      "  listener(msg);"
      "});"
      "exports.isExtensionRunningOnUIThread = function(callback) {"
      "  listener = callback;"
      "  extension.postMessage('');"
      "};"
      "exports.syncIsExtensionRunningOnUIThread = function() {"
      "  return extension.internal.sendSyncMessage('');"
      "};");
  }

  XWalkExtensionInstance* CreateInstance() {
    return new InProcessExtensionInstance();
  }
};

class InProcessThreadsTest : public XWalkExtensionsTestBase {
 public:
  virtual void RegisterExtensions(XWalkExtensionServer* server) OVERRIDE {
    bool registered = server->RegisterExtension(
        scoped_ptr<XWalkExtension>(
            new InProcessExtension(kInProcessUIThread)));
    ASSERT_TRUE(registered);
  }

  virtual void RegisterExtensionsOnExtensionThread(
      XWalkExtensionServer* server) OVERRIDE {
    bool registered = server->RegisterExtension(
        scoped_ptr<XWalkExtension>(
            new InProcessExtension(kInProcessExtensionThread)));
    ASSERT_TRUE(registered);
  }
};

IN_PROC_BROWSER_TEST_F(InProcessThreadsTest, InProcessThreads) {
  content::RunAllPendingInMessageLoop();

  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);

  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("in_process_threads.html"));
  xwalk_test_utils::NavigateToURL(runtime(), url);

  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
