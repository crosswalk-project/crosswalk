// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "base/values.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/application/common/manifest_handlers/tizen_metadata_handler.h"
#include "xwalk/application/common/manifest_handlers/unittest_util.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

namespace {

const TizenMetaDataInfo* GetInfo(scoped_refptr<ApplicationData> application) {
  const TizenMetaDataInfo* info = static_cast<TizenMetaDataInfo*>(
      application->GetManifestData(keys::kTizenMetaDataKey));
  return info;
}

scoped_ptr<base::DictionaryValue> CreateMetadata(const char* key,
                                                 const char* value) {
  scoped_ptr<base::DictionaryValue> metadata(new base::DictionaryValue);
  metadata->SetString(keys::kNamespaceKey, keys::kTizenNamespacePrefix);
  if (key)
    metadata->SetString(keys::kTizenMetaDataNameKey, key);
  if (value)
    metadata->SetString(keys::kTizenMetaDataValueKey, value);
  return metadata.Pass();
}

}  // namespace

class TizenMetadataHandlerTest: public testing::Test {
};

// Verifies Getters and Setters of TizenMetaDataInfo class
TEST_F(TizenMetadataHandlerTest, MetaDataInfoContent) {
  TizenMetaDataInfo tizenMetaDataInfo;
  EXPECT_FALSE(tizenMetaDataInfo.HasKey("key"));
  EXPECT_FALSE(tizenMetaDataInfo.HasKey("primaryKey"));
  tizenMetaDataInfo.SetValue("key", "value");
  tizenMetaDataInfo.SetValue("primaryKey", "importantValue");
  EXPECT_TRUE(tizenMetaDataInfo.HasKey("key"));
  EXPECT_TRUE(tizenMetaDataInfo.HasKey("primaryKey"));
  EXPECT_FALSE(tizenMetaDataInfo.HasKey("value"));
  EXPECT_FALSE(tizenMetaDataInfo.HasKey("importantValue"));
  EXPECT_FALSE(tizenMetaDataInfo.HasKey("xyz"));
  EXPECT_EQ(tizenMetaDataInfo.GetValue("key"), "value");
  EXPECT_EQ(tizenMetaDataInfo.GetValue("primaryKey"), "importantValue");
}

// Verifies parsing manifest doesn't containing any metadata info
TEST_F(TizenMetadataHandlerTest, NoMetaData) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::Type::TYPE_WIDGET, *manifest);
  EXPECT_NE(nullptr, application.get());
  const TizenMetaDataInfo* info = GetInfo(application);
  EXPECT_EQ(nullptr, info);
}

// Verifies parsing manifest containing one complete metadata key
TEST_F(TizenMetadataHandlerTest, OneMetaData) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();
  manifest->Set(keys::kTizenMetaDataKey,
                CreateMetadata("someKey", "someValue").release());
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::Type::TYPE_WIDGET, *manifest);
  EXPECT_NE(nullptr, application.get());
  const TizenMetaDataInfo* info = GetInfo(application);
  EXPECT_NE(nullptr, info);
  EXPECT_EQ(1, info->metadata().size());
}

// Verifies parsing manifest containing metadata key without value
TEST_F(TizenMetadataHandlerTest, MetaDataNoValue) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();
  manifest->Set(keys::kTizenMetaDataKey,
                CreateMetadata("someKey", nullptr).release());
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::Type::TYPE_WIDGET, *manifest);
  EXPECT_NE(nullptr, application.get());
  const TizenMetaDataInfo* info = GetInfo(application);
  EXPECT_NE(nullptr, info);
  EXPECT_EQ(1, info->metadata().size());
  EXPECT_EQ("someKey", info->metadata().begin()->first);
  EXPECT_TRUE(info->metadata().begin()->second.empty());
}

// Verifies parsing manifest containing no key
TEST_F(TizenMetadataHandlerTest, MetaDataNoKey) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();
  manifest->Set(keys::kTizenMetaDataKey,
                CreateMetadata(nullptr, "someValue").release());
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::Type::TYPE_WIDGET, *manifest);
  EXPECT_EQ(nullptr, application.get());
}

// Verifies parsing manifest containing empty key
TEST_F(TizenMetadataHandlerTest, MetaDataEmptyKey) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();
  manifest->Set(keys::kTizenMetaDataKey,
                CreateMetadata("", "someValue").release());
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::Type::TYPE_WIDGET, *manifest);
  EXPECT_EQ(nullptr, application.get());
}

// Verifies parsing manifest containing no key and no value
TEST_F(TizenMetadataHandlerTest, MetaDataNoKeyAndNoValue) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();
  manifest->Set(keys::kTizenMetaDataKey,
                CreateMetadata(nullptr, nullptr).release());
  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::Type::TYPE_WIDGET, *manifest);
  EXPECT_EQ(nullptr, application.get());
}

}  // namespace application
}  // namespace xwalk
