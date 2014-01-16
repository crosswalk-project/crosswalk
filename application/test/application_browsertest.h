// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TEST_APPLICATION_BROWSERTEST_H_
#define XWALK_APPLICATION_TEST_APPLICATION_BROWSERTEST_H_

#include "base/command_line.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/test/base/in_process_browser_test.h"

// Base class for application browser test.
class ApplicationBrowserTest: public InProcessBrowserTest {
 protected:
  ApplicationBrowserTest();
  virtual ~ApplicationBrowserTest() {}

  // Wait for Runtime number in RuntimeRegistry becomes |runtime_count|.
  void WaitForRuntimes(int runtime_count);

  int GetRuntimeCount() const;

  base::FilePath test_data_dir_;
};

#endif  // XWALK_APPLICATION_TEST_APPLICATION_BROWSERTEST_H_
