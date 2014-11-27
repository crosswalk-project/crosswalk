// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>
#include "base/memory/scoped_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/application/common/manifest_handlers/unittest_util.h"
#include "xwalk/application/common/manifest_handlers/tizen_navigation_handler.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

namespace {

const TizenNavigationInfo* GetNavigationInfo(
    scoped_refptr<ApplicationData> application) {
  const TizenNavigationInfo* info = static_cast<TizenNavigationInfo*>(
      application->GetManifestData(keys::kAllowNavigationKey));
  return info;
}

}  // namespace

class TizenNavigationHandlerTest: public testing::Test {
};

TEST_F(TizenNavigationHandlerTest, NoNavigation) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_WIDGET, *manifest);
  EXPECT_TRUE(application.get());
  EXPECT_FALSE(GetNavigationInfo(application));
}

TEST_F(TizenNavigationHandlerTest, OneNavigation) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();
  manifest->SetString(keys::kAllowNavigationKey, "http://www.sample.com");
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_WIDGET, *manifest);
  EXPECT_TRUE(application.get());
  EXPECT_EQ(application->GetManifest()->type(), Manifest::TYPE_WIDGET);
  const TizenNavigationInfo* info = GetNavigationInfo(application);
  EXPECT_TRUE(info);
  const std::vector<std::string>& list = info->GetAllowedDomains();
  EXPECT_TRUE(list.size() == 1 && list[0] == "http://www.sample.com");
}

TEST_F(TizenNavigationHandlerTest, Navigations) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();
  manifest->SetString(keys::kAllowNavigationKey,
                      "http://www.sample1.com www.sample2.com");
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_WIDGET, *manifest);
  EXPECT_TRUE(application.get());
  EXPECT_EQ(application->GetManifest()->type(), Manifest::TYPE_WIDGET);
  const TizenNavigationInfo* info = GetNavigationInfo(application);
  EXPECT_TRUE(info);
  const std::vector<std::string>& list = info->GetAllowedDomains();
  EXPECT_TRUE(list.size() == 2 &&
              list[0] == "http://www.sample1.com" &&
              list[1] == "www.sample2.com");
}

}  // namespace application
}  // namespace xwalk
