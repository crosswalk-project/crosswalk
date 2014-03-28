// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/widget_handler.h"

#include "xwalk/application/common/application_manifest_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

namespace {
// Below key names are readable from Javascript widget interface.
const char kWidgetAuthor[] = "author";
const char kWidgetDecription[] = "description";
const char kWidgetName[] = "name";
const char kWidgetShortName[] = "shortName";
const char kWidgetVersion[] = "version";
const char kWidgetID[] = "id";
const char kWidgetAuthorEmail[] = "authorEmail";
const char kWidgetAuthorHref[] = "authorHref";
const char kWidgetHeight[] = "height";
const char kWidgetWidth[] = "width";
const char kWidgetPreferences[] = "preferences";

// Child keys inside 'preferences' key.
const char kWidgetPreferencesName[] = "name";
const char kWidgetPreferencesValue[] = "value";
const char kWidgetPreferencesReadonly[] = "readonly";

// Value to test
const char author[] = "Some one";
const char decription[] = "This is a test";
const char name[] = "widget handler unittest";
const char shortName[] = "whu";
const char version[] = "0.0.0.1";
const char ID[] = "iiiiiiiiiid";
const char authorEmail[] = "aaa@bbb.com";
const char authorHref[] = "http://www.ssss.com";
const char height[] = "800";
const char width[] = "480";

const char* preferencesName[] = {"pname0", "pname1", "pname2"};
const char* preferencesValue[] = {"pvalue0", "pvalue1", "pvalue2"};
const char* preferencesReadonly[] = {"true", "false", "false"};
}  // namespace

class WidgetHandlerTest: public testing::Test {
 public:
  scoped_refptr<ApplicationData> CreateApplication(
    base::DictionaryValue &manifest) {
    std::string error;
    scoped_refptr<ApplicationData> application = ApplicationData::Create(
        base::FilePath(), Manifest::INVALID_TYPE, manifest, "", &error);
    return application;
  }

  WidgetInfo* GetWidgetInfo(scoped_refptr<ApplicationData> application) {
    WidgetInfo* info = static_cast<WidgetInfo*>(
      application->GetManifestData(keys::kWidgetKey));
    return info;
  }

  base::DictionaryValue* GetPreferencesItem(int id,
                                            bool is_parsed_manifest_key) {
    base::DictionaryValue* preferences = new base::DictionaryValue;
    if (is_parsed_manifest_key) {
      preferences->SetString(keys::kPreferencesNameKey,
                             preferencesName[id]);
      preferences->SetString(keys::kPreferencesValueKey,
                             preferencesValue[id]);
      // PreferencesReadonly is string on manifest and bool on widgetInfo
      preferences->SetString(keys::kPreferencesReadonlyKey,
                             preferencesReadonly[id]);
    } else {
      preferences->SetString(kWidgetPreferencesName,
                             preferencesName[id]);
      preferences->SetString(kWidgetPreferencesValue,
                             preferencesValue[id]);
      preferences->SetBoolean(kWidgetPreferencesReadonly,
                              strncmp(preferencesReadonly[id], "true", 4) == 0);
    }
    return preferences;
  }

  // No Preferences and full other information
  void SetAllInfoToManifest(base::DictionaryValue* manifest) {
    // Insert some key-value pairs into manifest use full key
    manifest->SetString(keys::kAuthorKey,      author);
    manifest->SetString(keys::kDescriptionKey, decription);
    manifest->SetString(keys::kNameKey,        name);
    manifest->SetString(keys::kShortNameKey,   shortName);
    manifest->SetString(keys::kVersionKey,     version);
    manifest->SetString(keys::kIDKey,          ID);
    manifest->SetString(keys::kAuthorEmailKey, authorEmail);
    manifest->SetString(keys::kAuthorHrefKey,  authorHref);
    manifest->SetString(keys::kHeightKey,      height);
    manifest->SetString(keys::kWidthKey,       width);
  }

  // No Preferences and full other information
  void SetAllInfoToWidget(base::DictionaryValue* widget) {
    // Insert some key-value pairs into widget use widget key;
    widget->SetString(kWidgetAuthor,      author);
    widget->SetString(kWidgetDecription,  decription);
    widget->SetString(kWidgetName,        name);
    widget->SetString(kWidgetShortName,   shortName);
    widget->SetString(kWidgetVersion,     version);
    widget->SetString(kWidgetID,          ID);
    widget->SetString(kWidgetAuthorEmail, authorEmail);
    widget->SetString(kWidgetAuthorHref,  authorHref);
    widget->SetString(kWidgetHeight,      height);
    widget->SetString(kWidgetWidth,       width);
  }
};

TEST_F(WidgetHandlerTest, ParseManifestWithOnlyNameAndVersion) {
  base::DictionaryValue manifest;
  manifest.SetString(keys::kNameKey, "no name");
  manifest.SetString(keys::kVersionKey, "0");

  scoped_refptr<ApplicationData> application = CreateApplication(manifest);
  EXPECT_TRUE(application);

  WidgetInfo* info = GetWidgetInfo(application);
  int size = info->GetWidgetInfo()->size();

  // Only name and version ,others are empty string "",but exist.
  // And widget have 10 items.
  EXPECT_EQ(size, 10);

  base::DictionaryValue* widget = info->GetWidgetInfo();
  base::DictionaryValue::Iterator it(*widget);

  std::string tmpStr;
  // Check value
  while (!it.IsAtEnd()) {
    it.value().GetAsString(&tmpStr);
    if (it.key() == kWidgetName) {
      EXPECT_EQ(tmpStr, "no name");
    } else if (it.key() == kWidgetVersion) {
      EXPECT_EQ(tmpStr, "0");
    } else {
      EXPECT_EQ(tmpStr, "");
    }
    it.Advance();
  }
}

TEST_F(WidgetHandlerTest,
       ParseManifestWithAllOfOtherItemsAndOnePreferenceItem) {
  // Create a manifest with one preference item.
  scoped_ptr<base::DictionaryValue> manifest(new base::DictionaryValue);
  SetAllInfoToManifest(manifest.get());
  manifest->Set(keys::kPreferencesKey, GetPreferencesItem(0, true));
  // Create an application use this manifest.
  scoped_refptr<ApplicationData> application;
  application = CreateApplication(*(manifest.get()));
  EXPECT_TRUE(application);
  EXPECT_EQ(application->GetPackageType(), Manifest::TYPE_WGT);
  // Get widget info from this application.
  WidgetInfo* info = GetWidgetInfo(application);
  EXPECT_TRUE(info);
  scoped_ptr<base::DictionaryValue> Copy(info->GetWidgetInfo()->DeepCopy());
  base::DictionaryValue* widget_parsed_from_manifest;
  Copy->GetAsDictionary(&widget_parsed_from_manifest);
  EXPECT_TRUE(widget_parsed_from_manifest);

  // Create a widget with one preference item manually.
  scoped_ptr<base::DictionaryValue> widget(new base::DictionaryValue);
  SetAllInfoToWidget(widget.get());
  widget->Set(kWidgetPreferences, GetPreferencesItem(0, false));

  // Compare the widget parsed from manifest with
  // the widget create manually.
  EXPECT_TRUE(widget->Equals(widget_parsed_from_manifest));
}

TEST_F(WidgetHandlerTest,
       ParseManifestWithAllOfOtherItemsAndThreePreferenceItemsList) {
  // Create a manifest with three preference items.
  scoped_ptr<base::DictionaryValue> manifest(new base::DictionaryValue);
  SetAllInfoToManifest(manifest.get());
  base::ListValue* manifestPreferences = new base::ListValue;
  for (int i = 0; i < 3; i++) {
    manifestPreferences->Append(GetPreferencesItem(i, true));
  }
  // Create an application use this manifest,
  scoped_refptr<ApplicationData> application;
  application = CreateApplication(*(manifest.get()));
  EXPECT_TRUE(application);
  EXPECT_EQ(application->GetPackageType(), Manifest::TYPE_WGT);
  // Get widget info from this application.
  WidgetInfo* info = GetWidgetInfo(application);
  EXPECT_TRUE(info);
  scoped_ptr<base::DictionaryValue> Copy(info->GetWidgetInfo()->DeepCopy());
  base::DictionaryValue* widget_parsed_from_manifest;
  Copy->GetAsDictionary(&widget_parsed_from_manifest);
  EXPECT_TRUE(widget_parsed_from_manifest);

  // Create a widget with three preference items manually.
  scoped_ptr<base::DictionaryValue> widget(new base::DictionaryValue);
  SetAllInfoToWidget(widget.get());
  base::ListValue* widgetPreferences   = new base::ListValue;
  for (int i = 0; i < 3; i++) {
    widgetPreferences->Append(GetPreferencesItem(i, false));
  }

  // Compare the widget parsed from manifest with
  // the widget create manually.
  EXPECT_TRUE(widget->Equals(widget_parsed_from_manifest));
}

}  // namespace application
}  // namespace xwalk
