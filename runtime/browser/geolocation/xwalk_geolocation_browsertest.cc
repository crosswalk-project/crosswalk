// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/path_service.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_registry.h"
#include "xwalk/runtime/common/xwalk_notification_types.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "testing/gmock/include/gmock/gmock.h"

using xwalk::NativeAppWindow;
using xwalk::Runtime;
using xwalk::RuntimeRegistry;
using xwalk::RuntimeList;
using content::WebContents;
using testing::_;

// A mock observer to listen runtime registry changes.
class MockRuntimeRegistryObserver : public xwalk::RuntimeRegistryObserver {
 public:
  MockRuntimeRegistryObserver() {}
  virtual ~MockRuntimeRegistryObserver() {}

  MOCK_METHOD1(OnRuntimeAdded, void(Runtime* runtime));
  MOCK_METHOD1(OnRuntimeRemoved, void(Runtime* runtime));

 private:
  DISALLOW_COPY_AND_ASSIGN(MockRuntimeRegistryObserver);
};

class XWalkRuntimeTest : public InProcessBrowserTest {
 public:
  XWalkRuntimeTest() {}
  virtual ~XWalkRuntimeTest() {
    original_runtimes_.clear();
    notification_observer_.reset();
  }

  // SetUpOnMainThread is called after BrowserMainRunner was initialized and
  // just before RunTestOnMainThread (aka. TestBody).
  virtual void SetUpOnMainThread() OVERRIDE {
    notification_observer_.reset(
        new content::WindowedNotificationObserver(
          xwalk::NOTIFICATION_RUNTIME_OPENED,
          content::NotificationService::AllSources()));
    const RuntimeList& runtimes = RuntimeRegistry::Get()->runtimes();
    for (RuntimeList::const_iterator it = runtimes.begin();
         it != runtimes.end(); ++it)
      original_runtimes_.push_back(*it);
  }

 private:
  RuntimeList original_runtimes_;
  scoped_ptr<content::WindowedNotificationObserver> notification_observer_;
};

IN_PROC_BROWSER_TEST_F(XWalkRuntimeTest, LoadGeolocationPage) {
  GURL url = xwalk_test_utils::GetTestURL(
      base::FilePath(), base::FilePath().AppendASCII(
          "geolocation/simple.html"));
  size_t len = RuntimeRegistry::Get()->runtimes().size();
  xwalk_test_utils::NavigateToURL(runtime(), url);
  int error_code;
  ASSERT_TRUE(content::ExecuteScriptAndExtractInt(
      runtime()->web_contents(),
      "console.log('wait for result...');",
      &error_code));
  EXPECT_NE(error_code, -1);
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len, RuntimeRegistry::Get()->runtimes().size());
  runtime()->Close();
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len - 1, RuntimeRegistry::Get()->runtimes().size());
}
