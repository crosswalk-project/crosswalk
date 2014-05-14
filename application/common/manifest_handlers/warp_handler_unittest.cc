// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/warp_handler.h"

#include "xwalk/application/common/application_manifest_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

class WARPHandlerTest: public testing::Test {
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

  const WARPInfo* GetWARPInfo(
      scoped_refptr<ApplicationData> application) {
    const WARPInfo* info = static_cast<WARPInfo*>(
        application->GetManifestData(keys::kAccessKey));
    return info;
  }

  base::DictionaryValue manifest;
};

// FIXME: the default WARP policy settings in WARP manifest handler
// are temporally removed, since they had affected some tests and legacy apps.
TEST_F(WARPHandlerTest, NoWARP) {
  scoped_refptr<ApplicationData> application = CreateApplication();
  EXPECT_TRUE(application.get());
  EXPECT_FALSE(GetWARPInfo(application));
}

TEST_F(WARPHandlerTest, OneWARP) {
  base::DictionaryValue* warp = new base::DictionaryValue;
  warp->SetString(keys::kAccessOriginKey, "http://www.sample.com");
  warp->SetBoolean(keys::kAccessSubdomainsKey, true);
  manifest.Set(keys::kAccessKey, warp);
  scoped_refptr<ApplicationData> application = CreateApplication();
  EXPECT_TRUE(application.get());
  EXPECT_EQ(application->GetPackageType(), Package::WGT);
  const WARPInfo* info = GetWARPInfo(application);
  EXPECT_TRUE(info);
  scoped_ptr<base::ListValue> list(info->GetWARP()->DeepCopy());
  base::DictionaryValue* new_warp;
  list->GetDictionary(0, &new_warp);
  EXPECT_TRUE(new_warp);
  EXPECT_TRUE(warp->Equals(new_warp));
}

TEST_F(WARPHandlerTest, WARPs) {
  base::DictionaryValue* warp1 = new base::DictionaryValue;
  warp1->SetString(keys::kAccessOriginKey, "http://www.sample1.com");
  warp1->SetBoolean(keys::kAccessSubdomainsKey, true);
  base::DictionaryValue* warp2 = new base::DictionaryValue;
  warp2->SetString(keys::kAccessOriginKey, "http://www.sample2.com");
  warp2->SetBoolean(keys::kAccessSubdomainsKey, false);
  base::ListValue* warp_list = new base::ListValue;
  warp_list->Append(warp1);
  warp_list->Append(warp2);
  manifest.Set(keys::kAccessKey, warp_list);

  scoped_refptr<ApplicationData> application = CreateApplication();
  EXPECT_TRUE(application.get());
  EXPECT_EQ(application->GetPackageType(), Package::WGT);

  const WARPInfo* info = GetWARPInfo(application);
  EXPECT_TRUE(info);

  scoped_ptr<base::ListValue> list(info->GetWARP()->DeepCopy());

  base::DictionaryValue* new_warp1;
  list->GetDictionary(0, &new_warp1);
  EXPECT_TRUE(new_warp1);
  base::DictionaryValue* new_warp2;
  list->GetDictionary(1, &new_warp2);
  EXPECT_TRUE(new_warp2);

  EXPECT_TRUE(warp1->Equals(new_warp1));
  EXPECT_TRUE(warp2->Equals(new_warp2));
}

}  // namespace application
}  // namespace xwalk
