// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TEST_APPLICATION_BROWSERTEST_H_
#define XWALK_APPLICATION_TEST_APPLICATION_BROWSERTEST_H_

#include "base/command_line.h"
#include "xwalk/test/base/in_process_browser_test.h"

// Base class for application browser test.
// TODO(xiang): Currently we don't support shared browser process model then
// every time we test an app we need pass its path in command line. Should
// provide load/unload, install/uninstall, launch support when shared browser
// process model is supported.
class ApplicationBrowserTest: public InProcessBrowserTest {
 protected:
  ApplicationBrowserTest();
  virtual ~ApplicationBrowserTest() {}

  // InProcessBrowserTest implementation.
  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE;

  // Wait for Runtime number in RuntimeRegistry becomes |runtime_count|.
  void WaitForRuntimes(int runtime_count);

  int GetRuntimeNumber();

  base::FilePath test_data_dir_;
};

#endif  // XWALK_APPLICATION_TEST_APPLICATION_BROWSERTEST_H_
