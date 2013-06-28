// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_TEST_BASE_IN_PROCESS_BROWSER_TEST_H_
#define CAMEO_TEST_BASE_IN_PROCESS_BROWSER_TEST_H_

#include "base/compiler_specific.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/common/page_transition_types.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_base.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace cameo {
class Runtime;
}

class CommandLine;

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
class InProcessBrowserTest : public content::BrowserTestBase {
 public:
  InProcessBrowserTest();
  virtual ~InProcessBrowserTest();

  // Configures everything for an in process browser test, then invokes
  // BrowserMain. BrowserMain ends up invoking RunTestOnMainThreadLoop.
  virtual void SetUp() OVERRIDE;

  // Restores state configured in SetUp.
  virtual void TearDown() OVERRIDE;

 protected:
  // Returns the runtime instance created by CreateRuntime.
  cameo::Runtime* runtime() const { return runtime_; }

  // Override this to add any custom cleanup code that needs to be done on the
  // main thread before the browser is torn down.
  virtual void CleanUpOnMainThread() {}

  // BrowserTestBase:
  virtual void RunTestOnMainThreadLoop() OVERRIDE;

  // Return a CommandLine object that is used to relaunch the browser_test
  // binary as a browser process.
  CommandLine GetCommandLineForRelaunch();

 private:
  // Create data path directory for this test to avoid pollution in default
  // data path. Return true if success.
  bool CreateDataPathDir();

  // Quits all open runtimes and waits until there are no more runtimes.
  void QuitAllRuntimes();

  // Prepare command line that will be used to launch the child browser process
  // with an in-process test.
  void PrepareTestCommandLine(CommandLine* command_line);

  // Browser created from CreateBrowser.
  cameo::Runtime* runtime_;

  // Temporary data path directory. Used only when a data path directory is not
  // specified in the command line.
  base::ScopedTempDir temp_data_path_dir_;
};

#endif  // CAMEO_TEST_BASE_IN_PROCESS_BROWSER_TEST_H_
