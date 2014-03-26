// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/base/xwalk_test_suite.h"

#include "xwalk/runtime/common/xwalk_content_client.h"

XWalkTestSuite::XWalkTestSuite(int argc, char** argv)
    : content::ContentTestSuiteBase(argc, argv) {
}

XWalkTestSuite::~XWalkTestSuite() {}

void XWalkTestSuite::Initialize() {
  // Initialize after overriding paths as some content paths depend on correct
  // values for DIR_EXE and DIR_MODULE.
  content::ContentTestSuiteBase::Initialize();

  {
    xwalk::XWalkContentClient client;
    content::ContentTestSuiteBase::RegisterContentSchemes(&client);
  }
}
