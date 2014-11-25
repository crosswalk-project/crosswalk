// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/scoped_ptr.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/unittest_util.h"
#include "xwalk/application/common/manifest_handlers/warp_handler.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

namespace {

const WARPInfo* GetWARPInfo(
    scoped_refptr<ApplicationData> application) {
  const WARPInfo* info = static_cast<WARPInfo*>(
      application->GetManifestData(keys::kAccessKey));
  return info;
}

}  // namespace

class WARPHandlerTest: public testing::Test {
};

// FIXME: the default WARP policy settings in WARP manifest handler
// are temporally removed, since they had affected some tests and legacy apps.
TEST_F(WARPHandlerTest, NoWARP) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_WIDGET, *manifest);
  EXPECT_TRUE(application.get());
  EXPECT_FALSE(GetWARPInfo(application));
}

TEST_F(WARPHandlerTest, OneWARP) {
  base::DictionaryValue* warp = new base::DictionaryValue;
  warp->SetString(keys::kAccessOriginKey, "http://www.sample.com");
  warp->SetBoolean(keys::kAccessSubdomainsKey, true);
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();
  manifest->Set(keys::kAccessKey, warp);
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_WIDGET, *manifest);
  EXPECT_TRUE(application.get());
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

  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();
  manifest->Set(keys::kAccessKey, warp_list);
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_WIDGET, *manifest);
  EXPECT_TRUE(application.get());

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
