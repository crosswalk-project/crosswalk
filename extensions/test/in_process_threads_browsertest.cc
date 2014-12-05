// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/browser/browser_thread.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/test/xwalk_extensions_test_base.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using namespace xwalk::extensions;  // NOLINT
using xwalk::Runtime;

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

    return reply.Pass();
  }

  void HandleMessage(scoped_ptr<base::Value> msg) override {
    PostMessageToJS(InRunningOnUIThread());
  }

  void HandleSyncMessage(scoped_ptr<base::Value> msg) override {
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

  XWalkExtensionInstance* CreateInstance() override {
    return new InProcessExtensionInstance();
  }
};

class InProcessThreadsTest : public XWalkExtensionsTestBase {
 public:
  void CreateExtensionsForUIThread(
      XWalkExtensionVector* extensions) override {
    extensions->push_back(new InProcessExtension(kInProcessUIThread));
  }

  void CreateExtensionsForExtensionThread(
      XWalkExtensionVector* extensions) override {
    extensions->push_back(new InProcessExtension(kInProcessExtensionThread));
  }
};

IN_PROC_BROWSER_TEST_F(InProcessThreadsTest, InProcessThreads) {
  Runtime* runtime = CreateRuntime();
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);

  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("in_process_threads.html"));
  xwalk_test_utils::NavigateToURL(runtime, url);

  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
