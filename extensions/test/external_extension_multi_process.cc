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

  // This will be called everytime a new RenderProcess has been created.
  void CreateExtensionsForUIThread(
      XWalkExtensionVector* extensions) override {
    register_extensions_count_++;
  }

  int CountRegisterExtensions() {
    return register_extensions_count_;
  }

 private:
  int register_extensions_count_;
};

IN_PROC_BROWSER_TEST_F(ExternalExtensionMultiProcessTest,
    OpenLinkInNewRuntimeAndSameRP) {
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII("same_rp.html"));
  Runtime* runtime = CreateRuntime(url);
  size_t len = runtimes().size();
  EXPECT_EQ(1, CountRegisterExtensions());

  SimulateMouseClick(runtime->web_contents(), 0,
      blink::WebMouseEvent::ButtonLeft);
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len + 1, runtimes().size());
  Runtime* second = runtimes().back();
  EXPECT_TRUE(NULL != second);
  EXPECT_NE(runtime, second);
  EXPECT_EQ(1, CountRegisterExtensions());
}

IN_PROC_BROWSER_TEST_F(ExternalExtensionMultiProcessTest,
    OpenLinkInNewRuntimeAndNewRP) {
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII("new_rp.html"));
  Runtime* runtime = CreateRuntime(url);
  size_t len = runtimes().size();
  EXPECT_EQ(1, CountRegisterExtensions());

  SimulateMouseClick(runtime->web_contents(), 0,
      blink::WebMouseEvent::ButtonLeft);
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len + 1, runtimes().size());
  Runtime* second = runtimes().back();
  EXPECT_TRUE(NULL != second);
  EXPECT_NE(runtime, second);
  EXPECT_EQ(2, CountRegisterExtensions());
}

IN_PROC_BROWSER_TEST_F(ExternalExtensionMultiProcessTest,
    CreateNewRuntimeAndNewRP) {
  GURL url = GetExtensionsTestURL(base::FilePath(),
                                  base::FilePath().AppendASCII("new_rp.html"));
  Runtime* runtime = CreateRuntime(url);
  size_t len = runtimes().size();
  EXPECT_EQ(1, CountRegisterExtensions());

  Runtime* new_runtime = CreateRuntime(url);
  EXPECT_NE(runtime, new_runtime);
  EXPECT_EQ(len + 1, runtimes().size());
  EXPECT_EQ(2, CountRegisterExtensions());
}
