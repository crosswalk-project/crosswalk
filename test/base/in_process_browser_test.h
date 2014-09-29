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

class RuntimeRegistry : public xwalk::Runtime::Observer {
 public:
  typedef std::vector<xwalk::Runtime*> RuntimeList;
  RuntimeRegistry();
  virtual ~RuntimeRegistry();

  void CloseAll();
  const RuntimeList& runtimes() const { return runtimes_; }

 private:
  virtual void OnRuntimeAdded(xwalk::Runtime* runtime) OVERRIDE;
  virtual void OnRuntimeRemoved(xwalk::Runtime* runtime) OVERRIDE;

  RuntimeList runtimes_;
};

// Base class for tests wanting to bring up a runtime (aka. browser) in the
// unit test process.
//
// Reference comments in chrome/test/base/in_process_browser_test.h file
// about how to write a InProcessBrowserTest.
//
class InProcessBrowserTest : public content::BrowserTestBase {
 public:
  typedef RuntimeRegistry::RuntimeList RuntimeList;

  InProcessBrowserTest();
  virtual ~InProcessBrowserTest();

  // Configures everything for an in process browser test, then invokes
  // BrowserMain. BrowserMain ends up invoking RunTestOnMainThreadLoop.
  virtual void SetUp() OVERRIDE;

 protected:
  // FIXME : Two following methods should be removed!
  xwalk::Runtime* runtime() const { return runtime_; }
  const RuntimeList& runtimes() const;
  // Use this as an observer when create a 'Runtime' instance within a test.
  RuntimeRegistry* runtime_registry() const {
      return runtime_registry_.get(); }

  xwalk::RuntimeContext* GetRuntimeContext() const;

  // Override this to add any custom cleanup code that needs to be done on the
  // main thread before the browser is torn down.
  virtual void ProperMainThreadCleanup() {}

  // BrowserTestBase:
  virtual void RunTestOnMainThreadLoop() OVERRIDE;

  // Return a CommandLine object that is used to relaunch the browser_test
  // binary as a browser process.
  base::CommandLine GetCommandLineForRelaunch();

 private:
  // Create data path directory for this test to avoid pollution in default
  // data path. Return true if success.
  bool CreateDataPathDir();

  // Prepare command line that will be used to launch the child browser process
  // with an in-process test.
  void PrepareTestCommandLine(base::CommandLine* command_line);

  scoped_ptr<RuntimeRegistry> runtime_registry_;
  // FIXME : Should be removed.
  xwalk::Runtime* runtime_;

  // Temporary data path directory. Used only when a data path directory is not
  // specified in the command line.
  base::ScopedTempDir temp_data_path_dir_;
};

#endif  // XWALK_TEST_BASE_IN_PROCESS_BROWSER_TEST_H_
