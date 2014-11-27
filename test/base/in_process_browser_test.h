// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_BASE_IN_PROCESS_BROWSER_TEST_H_
#define XWALK_TEST_BASE_IN_PROCESS_BROWSER_TEST_H_

#include <vector>

#include "base/compiler_specific.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_base.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/page_transition_types.h"
#include "xwalk/runtime/browser/runtime.h"

namespace base {
class CommandLine;
}

namespace content {
class ContentRendererClient;
}

namespace net {
class RuleBasedHostResolverProc;
}

// Base class for tests wanting to bring up a runtime (aka. browser) in the
// unit test process.
//
// Reference comments in chrome/test/base/in_process_browser_test.h file
// about how to write a InProcessBrowserTest.
//
class InProcessBrowserTest : public content::BrowserTestBase,
                             public xwalk::Runtime::Observer {
 public:
  using RuntimeList = std::vector<xwalk::Runtime*>;

  InProcessBrowserTest();
  virtual ~InProcessBrowserTest();

  // Configures everything for an in process browser test, then invokes
  // BrowserMain. BrowserMain ends up invoking RunTestOnMainThreadLoop.
  virtual void SetUp() override;

 protected:
  const RuntimeList& runtimes() const { return runtimes_.get(); }

  xwalk::Runtime* CreateRuntime(
      const GURL& url = GURL(),
      const xwalk::NativeAppWindow::CreateParams& params =
        xwalk::NativeAppWindow::CreateParams());

  // Override this to add any custom cleanup code that needs to be done on the
  // main thread before the browser is torn down.
  virtual void ProperMainThreadCleanup() {}

  // BrowserTestBase:
  virtual void RunTestOnMainThreadLoop() override;

 private:
  // xwalk::Runtime::Observer
  virtual void OnNewRuntimeAdded(xwalk::Runtime* runtime) override;
  virtual void OnRuntimeClosed(xwalk::Runtime* runtime) override;

  void CloseAll();
  // Create data path directory for this test to avoid pollution in default
  // data path. Return true if success.
  bool CreateDataPathDir();

  ScopedVector<xwalk::Runtime> runtimes_;

  // Temporary data path directory. Used only when a data path directory is not
  // specified in the command line.
  base::ScopedTempDir temp_data_path_dir_;
};

#endif  // XWALK_TEST_BASE_IN_PROCESS_BROWSER_TEST_H_
