// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_service.h"

#include "base/basictypes.h"
#include "testing/gtest/include/gtest/gtest.h"

using xwalk::extensions::ValidateExtensionNameForTesting;

TEST(XWalkExtensionServiceTest, ValidateExtensionName) {
  const std::string valid_names[] = {
    "xwalk",
    "xwalk.experimental",
    "xwalk.experimental.dialog",
    "my_extension",
  };

  const std::string invalid_names[] = {
    "",
    " ",
    "xwalk ",
    "xwalk..experimental",
    "1xwalk",
    ".",
    "'",
    "&",
    "$",
    "xwalk()",
    "xwalk.something.",
    "_something",
  };

  for (size_t i = 0; i < arraysize(valid_names); ++i) {
    EXPECT_TRUE(ValidateExtensionNameForTesting(valid_names[i]))
        << "Extension name should be valid: " << valid_names[i];
  }

  for (size_t i = 0; i < arraysize(invalid_names); ++i) {
    EXPECT_FALSE(ValidateExtensionNameForTesting(invalid_names[i]))
        << "Extension name should be invalid: " << invalid_names[i];
  }
}
