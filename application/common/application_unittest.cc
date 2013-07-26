// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/file_util.h"
#include "base/path_service.h"
#include "xwalk/application/common/application.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/application/common/id_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace xwalk_application {

// We persist location values in the preferences, so this is a sanity test that
// someone doesn't accidentally change them.
TEST(ApplicationTest, LocationValuesTest) {
  ASSERT_EQ(0, Manifest::INVALID_LOCATION);
  ASSERT_EQ(1, Manifest::UNPACKED);
}

TEST(ApplicationTest, IdIsValid) {
  EXPECT_TRUE(Application::IdIsValid("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  EXPECT_TRUE(Application::IdIsValid("pppppppppppppppppppppppppppppppp"));
  EXPECT_TRUE(Application::IdIsValid("abcdefghijklmnopabcdefghijklmnop"));
  EXPECT_TRUE(Application::IdIsValid("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"));
  EXPECT_FALSE(Application::IdIsValid("abcdefghijklmnopabcdefghijklmno"));
  EXPECT_FALSE(Application::IdIsValid("abcdefghijklmnopabcdefghijklmnopa"));
  EXPECT_FALSE(Application::IdIsValid("0123456789abcdef0123456789abcdef"));
  EXPECT_FALSE(Application::IdIsValid("abcdefghijklmnopabcdefghijklmnoq"));
  EXPECT_FALSE(Application::IdIsValid("abcdefghijklmnopabcdefghijklmno0"));
}

}  // namespace xwalk_application
