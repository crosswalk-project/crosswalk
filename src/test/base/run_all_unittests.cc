// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/test/base/cameo_test_suite.h"
#include "content/public/test/unittest_test_suite.h"

int main(int argc, char **argv) {
  return content::UnitTestTestSuite(new CameoTestSuite(argc, argv)).Run();
}
