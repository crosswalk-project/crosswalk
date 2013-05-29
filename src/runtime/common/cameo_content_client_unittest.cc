// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/runtime/common/cameo_content_client.h"

#include "base/command_line.h"
#include "base/strings/string_split.h"
#include "content/public/common/content_switches.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

void CheckUserAgentStringOrdering(bool mobile_device) {
  std::vector<std::string> pieces;

  // Check if the pieces of the user agent string come in the correct order.
  cameo::CameoContentClient content_client;
  std::string buffer = content_client.GetUserAgent();

  base::SplitStringUsingSubstr(buffer, "Mozilla/5.0 (", &pieces);
  ASSERT_EQ(2u, pieces.size());
  buffer = pieces[1];
  EXPECT_EQ("", pieces[0]);

  base::SplitStringUsingSubstr(buffer, ") AppleWebKit/", &pieces);
  ASSERT_EQ(2u, pieces.size());
  buffer = pieces[1];
  std::string os_str = pieces[0];

  base::SplitStringUsingSubstr(buffer, " (KHTML, like Gecko) ", &pieces);
  ASSERT_EQ(2u, pieces.size());
  buffer = pieces[1];
  std::string webkit_version_str = pieces[0];

  base::SplitStringUsingSubstr(buffer, " Safari/", &pieces);
  ASSERT_EQ(2u, pieces.size());
  std::string product_str = pieces[0];
  std::string safari_version_str = pieces[1];

  // Not sure what can be done to better check the OS string, since it's highly
  // platform-dependent.
  EXPECT_GT(os_str.size(), 0u);

  // Check that the version numbers match.
  EXPECT_GT(webkit_version_str.size(), 0u);
  EXPECT_GT(safari_version_str.size(), 0u);
  EXPECT_EQ(webkit_version_str, safari_version_str);

  EXPECT_EQ(0u, product_str.find("Chrome/"));
  if (mobile_device) {
    // "Mobile" gets tacked on to the end for mobile devices, like phones.
    const std::string kMobileStr = " Mobile";
    EXPECT_EQ(kMobileStr,
              product_str.substr(product_str.size() - kMobileStr.size()));
  }
}

}  // namespace

TEST(CameoContentClientTest, Basic) {
  CheckUserAgentStringOrdering(false);
}
