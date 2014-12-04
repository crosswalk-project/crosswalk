// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file_util.h"
#include "base/path_service.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/application/common/id_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace xwalk {
namespace application {

// We persist location values in the preferences, so this is a sanity test that
// someone doesn't accidentally change them.
TEST(ApplicationTest, LocationValuesTest) {
  ASSERT_EQ(0, ApplicationData::INTERNAL);
  ASSERT_EQ(1, ApplicationData::LOCAL_DIRECTORY);
  ASSERT_EQ(2, ApplicationData::TEMP_DIRECTORY);
  ASSERT_EQ(3, ApplicationData::EXTERNAL_URL);
}

}  // namespace application
}  // namespace xwalk
