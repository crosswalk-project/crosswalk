// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_BASE_XWALK_TEST_SUITE_H_
#define XWALK_TEST_BASE_XWALK_TEST_SUITE_H_

#include "content/public/test/content_test_suite_base.h"

class XWalkTestSuite : public content::ContentTestSuiteBase {
 public:
  XWalkTestSuite(int argc, char** argv);
  virtual ~XWalkTestSuite();

 protected:
  virtual void Initialize() OVERRIDE;
};

#endif  // XWALK_TEST_BASE_XWALK_TEST_SUITE_H_
