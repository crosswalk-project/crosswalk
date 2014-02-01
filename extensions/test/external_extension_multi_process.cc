// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/native_library.h"
#include "base/path_service.h"
#include "xwalk/extensions/test/xwalk_extensions_test_base.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/common/xwalk_notification_types.h"
#include "xwalk/test/base/xwalk_test_utils.h"
#include "content/public/browser/notification_service.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"

using xwalk::NativeAppWindow;
using xwalk::Runtime;
using xwalk::extensions::XWalkExtensionVector;

class ExternalExtensionMultiProcessTest : public XWalkExtensionsTestBase {
 public:
  ExternalExtensionMultiProcessTest()
    : register_extensions_count_(0) {}

  virtual ~ExternalExtensionMultiProcessTest() {
    original_runtimes_.clear();
    notification_observer_.reset();
  }

  // SetUpOnMainThread is called after BrowserMainRunner was initialized and
  // just before RunTestOnMainThread.
  virtual void SetUpOnMainThread() OVERRIDE {
    notification_observer_.reset(
        new content::WindowedNotificationObserver(
          xwalk::NOTIFICATION_RUNTIME_OPENED,
          content::NotificationService::AllSources()));
    original_runtimes_.assign(runtimes().begin(), runtimes().end());
  }

  // Block UI thread until a new Runtime instance is created.
  Runtime* WaitForSingleNewRuntime() {
    notification_observer_->Wait();
    for (RuntimeList::const_iterator it = runtimes().begin();
         it != runtimes().end(); ++it) {
      RuntimeList::iterator target =
          std::find(original_runtimes_.begin(), original_runtimes_.end(), *it);
      // Not found means a new one.
      if (target == original_runtimes_.end()) {
        original_runtimes_.push_back(*it);
        return *it;
      }
    }
    return NULL;
  }

  // This will be called everytime a new RenderProcess has been created.
  virtual void CreateExtensionsForUIThread(
      XWalkExtensionVector* extensions) OVERRIDE {
    register_extensions_count_++;
  }

  int CountRegisterExtensions() {
    return register_extensions_count_;
  }

 private:
  RuntimeList original_runtimes_;
  scoped_ptr<content::WindowedNotificationObserver> notification_observer_;

  int register_extensions_count_;
};

IN_PROC_BROWSER_TEST_F(ExternalExtensionMultiProcessTest,
    OpenLinkInNewRuntimeAndSameRP) {
  size_t len = runtimes().size();

  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII("same_rp.html"));

  xwalk_test_utils::NavigateToURL(runtime(), url);
  WaitForLoadStop(runtime()->web_contents());

  EXPECT_EQ(1, CountRegisterExtensions());

  SimulateMouseClick(runtime()->web_contents(), 0,
      WebKit::WebMouseEvent::ButtonLeft);
  content::RunAllPendingInMessageLoop();
  Runtime* second = WaitForSingleNewRuntime();
  EXPECT_TRUE(NULL != second);
  EXPECT_NE(runtime(), second);
  EXPECT_EQ(len + 1, runtimes().size());
  EXPECT_EQ(1, CountRegisterExtensions());
}

IN_PROC_BROWSER_TEST_F(ExternalExtensionMultiProcessTest,
    OpenLinkInNewRuntimeAndNewRP) {
  size_t len = runtimes().size();

  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII("new_rp.html"));

  xwalk_test_utils::NavigateToURL(runtime(), url);
  WaitForLoadStop(runtime()->web_contents());

  EXPECT_EQ(1, CountRegisterExtensions());

  SimulateMouseClick(runtime()->web_contents(), 0,
      WebKit::WebMouseEvent::ButtonLeft);
  content::RunAllPendingInMessageLoop();
  Runtime* second = WaitForSingleNewRuntime();
  EXPECT_TRUE(NULL != second);
  EXPECT_NE(runtime(), second);
  EXPECT_EQ(len + 1, runtimes().size());
  EXPECT_EQ(2, CountRegisterExtensions());
}

IN_PROC_BROWSER_TEST_F(ExternalExtensionMultiProcessTest,
    CreateNewRuntimeAndNewRP) {
  size_t len = runtimes().size();

  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII("new_rp.html"));

  xwalk_test_utils::NavigateToURL(runtime(), url);
  WaitForLoadStop(runtime()->web_contents());
  EXPECT_EQ(1, CountRegisterExtensions());

  Runtime* new_runtime = Runtime::CreateWithDefaultWindow(
      runtime()->runtime_context(), url, runtime_registry());
  EXPECT_EQ(new_runtime, WaitForSingleNewRuntime());
  EXPECT_NE(runtime(), new_runtime);
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len + 1, runtimes().size());
  EXPECT_EQ(2, CountRegisterExtensions());
}
