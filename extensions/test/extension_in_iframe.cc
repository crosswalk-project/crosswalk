// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/xwalk_extensions_test_base.h"

#include "base/synchronization/lock.h"
#include "base/synchronization/spin_wait.h"
#include "content/public/browser/web_contents.h"
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

base::Lock g_count_lock;
int g_count = 0;

}

class CounterExtensionContext : public XWalkExtensionInstance {
 public:
  CounterExtensionContext() {
  }

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {
    base::AutoLock lock(g_count_lock);
    g_count++;
  }
};

class CounterExtension : public XWalkExtension {
 public:
  CounterExtension()
      : XWalkExtension() {
    set_name("counter");
    set_javascript_api(
        "exports.count = function() {"
        "  extension.postMessage('PING');"
        "};");
  }

  virtual XWalkExtensionInstance* CreateInstance() {
    return new CounterExtensionContext();
  }
};

class XWalkExtensionsIFrameTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionServer* server) OVERRIDE {
    ASSERT_TRUE(RegisterExtensionForTest(server, new CounterExtension));
  }
};

IN_PROC_BROWSER_TEST_F(XWalkExtensionsIFrameTest,
                       ContextsAreCreatedForIFrames) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("counter_with_iframes.html"));
  xwalk_test_utils::NavigateToURL(runtime(), url);
  SPIN_FOR_1_SECOND_OR_UNTIL_TRUE(g_count == 3);
  ASSERT_EQ(g_count, 3);
}

IN_PROC_BROWSER_TEST_F(XWalkExtensionsIFrameTest,
                       ContextsAreNotCreatedForIFramesWithBlankPages) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("blank_iframes.html"));

  // We are mainly validating the fix for the issue #602. We first create a page
  // full of blank iframes and afterwards we navigate to another page.
  // ModuleSystems should not be created and consequentially not deleted for the
  // blank iframes.
  xwalk_test_utils::NavigateToURL(runtime(), url);
  content::TitleWatcher title_watcher1(runtime()->web_contents(), kPassString);
  xwalk_test_utils::NavigateToURL(runtime(), url);
  content::TitleWatcher title_watcher2(runtime()->web_contents(), kPassString);
}

// This test reproduces the problem found in bug
// https://github.com/crosswalk-project/crosswalk/issues/602. In summary, we
// were assuming that if at DidCreateScriptContext() a certain frame have URL
// "about:blank", that would also be true at WillReleaseScriptContext(), which
// is false.
//
// Using document.write into an empty document will change the URL for the frame
// we get in WillReleaseScriptContext(). The crash in the original code was due
// to not creating a module system for "about:blank" pages, and then trying to
// delete it when releasing the script context.
IN_PROC_BROWSER_TEST_F(XWalkExtensionsIFrameTest,
                       IFrameUsingDocumentWriteShouldNotCrash) {
  content::RunAllPendingInMessageLoop();
  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("iframe_using_document_write.html"));

  for (int i = 0; i < 5; i++) {
    content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
    xwalk_test_utils::NavigateToURL(runtime(), url);
    EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
  }
}

