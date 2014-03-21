// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/navigation_handler.h"

#include <string>
#include <vector>
#include "xwalk/application/common/application_manifest_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

class NavigationHandlerTest: public testing::Test {
 public:
  virtual void SetUp() OVERRIDE {
    manifest.SetString(keys::kNameKey, "no name");
    manifest.SetString(keys::kVersionKey, "0");
  }

  scoped_refptr<ApplicationData> CreateApplication() {
    std::string error;
    scoped_refptr<ApplicationData> application = ApplicationData::Create(
        base::FilePath(), Manifest::INVALID_TYPE, manifest, "", &error);
    return application;
  }

  const NavigationInfo* GetNavigationInfo(
      scoped_refptr<ApplicationData> application) {
    const NavigationInfo* info = static_cast<NavigationInfo*>(
        application->GetManifestData(keys::kAllowNavigationKey));
    return info;
  }

  base::DictionaryValue manifest;
};

TEST_F(NavigationHandlerTest, NoNavigation) {
  scoped_refptr<ApplicationData> application = CreateApplication();
  EXPECT_TRUE(application.get());
  EXPECT_FALSE(GetNavigationInfo(application));
}

TEST_F(NavigationHandlerTest, OneNavigation) {
  manifest.SetString(keys::kAllowNavigationKey, "http://www.sample.com");
  scoped_refptr<ApplicationData> application = CreateApplication();
  EXPECT_TRUE(application.get());
  EXPECT_EQ(application->GetPackageType(), Manifest::TYPE_WGT);
  const NavigationInfo* info = GetNavigationInfo(application);
  EXPECT_TRUE(info);
  const std::vector<std::string>& list = info->GetAllowedDomains();
  EXPECT_TRUE(list.size() == 1 && list[0] == "http://www.sample.com");
}

TEST_F(NavigationHandlerTest, Navigations) {
  manifest.SetString(keys::kAllowNavigationKey,
                     "http://www.sample1.com www.sample2.com");
  scoped_refptr<ApplicationData> application = CreateApplication();
  EXPECT_TRUE(application.get());
  EXPECT_EQ(application->GetPackageType(), Manifest::TYPE_WGT);
  const NavigationInfo* info = GetNavigationInfo(application);
  EXPECT_TRUE(info);
  const std::vector<std::string>& list = info->GetAllowedDomains();
  EXPECT_TRUE(list.size() == 2 &&
              list[0] == "http://www.sample1.com" &&
              list[1] == "www.sample2.com");
}

}  // namespace application
}  // namespace xwalk
