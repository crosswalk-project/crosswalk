// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TEST_APPLICATION_BROWSERTEST_H_
#define XWALK_APPLICATION_TEST_APPLICATION_BROWSERTEST_H_

#include "base/command_line.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/extensions/common/xwalk_extension_vector.h"
#include "xwalk/test/base/in_process_browser_test.h"

class ApiTestRunner;
// Base class for application browser test.
class ApplicationBrowserTest: public InProcessBrowserTest {
 protected:
  ApplicationBrowserTest();
  virtual ~ApplicationBrowserTest();

  virtual void SetUp() OVERRIDE;

  virtual void ProperMainThreadCleanup() OVERRIDE;

  xwalk::application::ApplicationService* application_sevice() const;

  scoped_ptr<ApiTestRunner> test_runner_;
  base::FilePath test_data_dir_;

 private:
  void CreateExtensions(xwalk::extensions::XWalkExtensionVector* extensions);
};

#endif  // XWALK_APPLICATION_TEST_APPLICATION_BROWSERTEST_H_
