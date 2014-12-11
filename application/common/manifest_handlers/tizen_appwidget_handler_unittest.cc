// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <string>
#include <vector>
#include "base/memory/ref_counted.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/values.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/application/common/manifest_handlers/tizen_appwidget_handler.h"
#include "xwalk/application/common/manifest_handlers/unittest_util.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

namespace {

// Extracts app-widget info object from application data object.
const TizenAppWidgetInfo* GetInfo(scoped_refptr<ApplicationData> application) {
  const TizenAppWidgetInfo* info = static_cast<TizenAppWidgetInfo*>(
      application->GetManifestData(keys::kTizenAppWidgetFullKey));
  return info;
}

// In specified dictionary sets a specified value under a specified key.
// The value is set only if it is not empty.
void SetNotEmptyString(const std::string& key, const std::string& value,
    base::DictionaryValue* dictionary) {
  CHECK(!key.empty());
  CHECK(dictionary);
  if (!value.empty())
    dictionary->SetString(key, value);
}

// Creates a dictionary with namespace set to tizen.
scoped_ptr<base::DictionaryValue> CreateTizenElement() {
  scoped_ptr<base::DictionaryValue> result(new base::DictionaryValue);
  result->SetString(keys::kNamespaceKey, keys::kTizenNamespacePrefix);
  return result.Pass();
}

// Creates a dictionary representing an app-widget element.
scoped_ptr<base::DictionaryValue> CreateAppWidget(
    const std::string& id, const std::string& primary,
    const std::string& update_peroid = std::string(),
    const std::string& auto_launch = std::string()) {
  scoped_ptr<base::DictionaryValue> result = CreateTizenElement();
  SetNotEmptyString(keys::kTizenAppWidgetIdKey, id, result.get());
  SetNotEmptyString(keys::kTizenAppWidgetPrimaryKey, primary, result.get());
  SetNotEmptyString(keys::kTizenAppWidgetUpdatePeriodKey,
                    update_peroid, result.get());
  SetNotEmptyString(keys::kTizenAppWidgetAutoLaunchKey,
                    auto_launch, result.get());
  return result.Pass();
}

// Creates a dictionary representing a box-label element.
scoped_ptr<base::DictionaryValue> CreateBoxLabel(
    const std::string& label, const std::string& lang = std::string()) {
  scoped_ptr<base::DictionaryValue> result = CreateTizenElement();
  SetNotEmptyString(keys::kTizenAppWidgetBoxLabelTextKey, label, result.get());
  SetNotEmptyString(keys::kTizenAppWidgetBoxLabelLangKey, lang, result.get());
  return result.Pass();
}

// Creates a dictionary representing a box-icon element.
scoped_ptr<base::DictionaryValue> CreateBoxIcon(const std::string& src) {
  scoped_ptr<base::DictionaryValue> result = CreateTizenElement();
  SetNotEmptyString(keys::kTizenAppWidgetBoxIconSrcKey, src, result.get());
  return result.Pass();
}

// Creates a dictionary representing a box-content element.
scoped_ptr<base::DictionaryValue> CreateBoxContent(
    const std::string& src, const std::string& mouse_event = std::string(),
    const std::string& touch_effect = std::string()) {
  scoped_ptr<base::DictionaryValue> result = CreateTizenElement();
  SetNotEmptyString(keys::kTizenAppWidgetBoxContentSrcKey,
                    src, result.get());
  SetNotEmptyString(keys::kTizenAppWidgetBoxContentMouseEventKey,
                    mouse_event, result.get());
  SetNotEmptyString(keys::kTizenAppWidgetBoxContentTouchEffectKey,
                    touch_effect, result.get());
  return result.Pass();
}

// Creates a dictionary representing a box-size element.
scoped_ptr<base::DictionaryValue> CreateBoxSize(
    const std::string& type, const std::string& preview = std::string(),
    const std::string& use_decoration = std::string()) {
  scoped_ptr<base::DictionaryValue> result = CreateTizenElement();
  SetNotEmptyString(keys::kTizenAppWidgetBoxContentSizeTextKey,
                    type, result.get());
  SetNotEmptyString(keys::kTizenAppWidgetBoxContentSizePreviewKey,
                    preview, result.get());
  SetNotEmptyString(keys::kTizenAppWidgetBoxContentSizeUseDecorationKey,
                    use_decoration, result.get());
  return result.Pass();
}

// Creates a dictionary representing a pd element.
scoped_ptr<base::DictionaryValue> CreateBoxDropView(
    const std::string& src, const std::string& width,
    const std::string& height) {
  scoped_ptr<base::DictionaryValue> result = CreateTizenElement();
  SetNotEmptyString(keys::kTizenAppWidgetBoxContentDropViewSrcKey,
                    src, result.get());
  SetNotEmptyString(keys::kTizenAppWidgetBoxContentDropViewWidthKey,
                    width, result.get());
  SetNotEmptyString(keys::kTizenAppWidgetBoxContentDropViewHeightKey,
                    height, result.get());
  return result.Pass();
}

// Creates an app-widget element id basing on values of default package id
// and default application name.
std::string CreateAppWidgetId(const std::string& name) {
  std::vector<std::string> parts;
  parts.push_back(kDefaultWidgetPackageId);
  parts.push_back(kDefaultWidgetApplicationName);
  parts.push_back(name);
  return JoinString(parts, '.');
}

// Creates a dictionary representing an app-widget element with all required
// fields and child elements set.
scoped_ptr<base::DictionaryValue> CreateMinimalAppWidget(
    const std::string& name, const std::string& primary) {
  std::string id = CreateAppWidgetId(name);

  scoped_ptr<base::DictionaryValue> result = CreateAppWidget(id, primary);

  CHECK(AddDictionary(keys::kTizenAppWidgetBoxLabelKey,
      CreateBoxLabel("sample label"), result.get()));

  scoped_ptr<base::DictionaryValue> box_content =
      CreateBoxContent("index.html");

  CHECK(AddDictionary(keys::kTizenAppWidgetBoxContentSizeKey,
      CreateBoxSize("1x1"), box_content.get()));

  CHECK(AddDictionary(keys::kTizenAppWidgetBoxContentKey,
      box_content.Pass(), result.get()));

  return result.Pass();
}

// Creates a dictionary representing an app-widget element with all fields and
// child elements set. Each optional field is set to something different than
// its default value if specified.
scoped_ptr<base::DictionaryValue> CreateFullAppWidget(
    const std::string& name, const std::string& primary) {
  std::string id = CreateAppWidgetId(name);

  scoped_ptr<base::DictionaryValue> result =
      CreateAppWidget(id, primary, "1800.0", "true");

  CHECK(AddDictionary(keys::kTizenAppWidgetBoxLabelKey,
      CreateBoxLabel("sample label"), result.get()));

  CHECK(AddDictionary(keys::kTizenAppWidgetBoxLabelKey,
      CreateBoxLabel("sample en label", "en"), result.get()));

  CHECK(AddDictionary(keys::kTizenAppWidgetBoxIconKey,
      CreateBoxIcon("icon.png"), result.get()));

  scoped_ptr<base::DictionaryValue> box_content =
      CreateBoxContent("index.html", "true", "false");

  CHECK(AddDictionary(keys::kTizenAppWidgetBoxContentSizeKey,
      CreateBoxSize("1x1"), box_content.get()));

  CHECK(AddDictionary(keys::kTizenAppWidgetBoxContentSizeKey,
      CreateBoxSize("2x1", "image.png"), box_content.get()));

  CHECK(AddDictionary(keys::kTizenAppWidgetBoxContentSizeKey,
      CreateBoxSize("2x2", "", "false"), box_content.get()));

  CHECK(AddDictionary(keys::kTizenAppWidgetBoxContentDropViewKey,
      CreateBoxDropView("index.html", "720", "150"), box_content.get()));

  CHECK(AddDictionary(keys::kTizenAppWidgetBoxContentKey,
      box_content.Pass(), result.get()));

  return result.Pass();
}

// Tests wheter specified value type is a dictionary or a list.
bool IsComplexValueType(base::Value::Type type) {
  return type == base::Value::TYPE_DICTIONARY || type == base::Value::TYPE_LIST;
}

// Searches for a child value specified by name in specified root value and
// if the child is found, fills child and parent arguments with the found child
// and its parent. Both parent and child arguments are optional.
bool GetParentAndChild(const std::string& name, base::Value* root,
    base::DictionaryValue** parent, base::Value** child) {
  CHECK(!name.empty());
  CHECK(root);

  base::DictionaryValue* tmp_parent = nullptr;
  if (!root->GetAsDictionary(&tmp_parent))
    return false;

  base::Value* tmp_child;
  if (!tmp_parent->Get(name, &tmp_child))
    return false;

  if (parent)
    *parent = tmp_parent;
  if (child)
    *child = tmp_child;

  return true;
}

// Searches for a child value specified by name and index in specified root
// value and if the child is found, fills child and parent arguments with
// the found child and its parent. Both parent and child arguments are optional.
bool GetParentAndChild(const std::string& name, size_t index,
    base::Value* root, base::ListValue** parent, base::Value** child) {
  CHECK(!name.empty());
  CHECK(root);

  base::Value* first_level_child;
  if (!GetParentAndChild(name, root, nullptr, &first_level_child))
    return false;

  base::ListValue* tmp_parent = nullptr;
  if (!first_level_child->GetAsList(&tmp_parent))
    return false;

  base::Value* tmp_child;
  if (!tmp_parent->Get(index, &tmp_child))
    return false;

  if (parent)
    *parent = tmp_parent;
  if (child)
    *child = tmp_child;

  return true;
}

// If the value argument is nullptr, the function removes the value specified by
// path, otherwise it changes the value specified by path to the value argument.
bool ChangeOrRemove(const std::string& path, base::Value * root,
    const std::string* value = nullptr) {
  CHECK(!path.empty());
  CHECK(root);

  base::Value* current = root;

  std::vector<std::string> parts;
  base::SplitString(path, '.', &parts);
  for (size_t i = 0, count = parts.size(); i < count; ++i) {
    const std::string& part = parts[i];

    std::vector<std::string> name_and_index;
    base::SplitString(part, ':', &name_and_index);
    if (name_and_index.size() == 1) {  // current part refers to a dictionary
      const std::string& name = name_and_index[0];

      base::DictionaryValue* parent;
      base::Value* child;
      if (!GetParentAndChild(name, current, &parent, &child))
        return false;

      if (i == count - 1) {
        if (value) {
          if (IsComplexValueType(child->GetType()))
            return false;
          parent->SetString(name, *value);
          return true;
        }
        return parent->Remove(name, nullptr);
      }

      current = child;
    } else if (name_and_index.size() == 2) {  // current part refers to a list
      const std::string& name = name_and_index[0];
      unsigned index;
      if (!base::StringToUint(name_and_index[1], &index))
        return false;  // invalid path

      base::ListValue* parent;
      base::Value* child;
      if (!GetParentAndChild(name, index, current, &parent, &child))
        return false;

      if (i == count - 1) {
        if (value) {
          if (IsComplexValueType(child->GetType()))
            return false;
          parent->Set(index, new base::StringValue(*value));
          return true;
        }
        return parent->Remove(index, nullptr);
      }

      current = child;
    } else {  // current part is invalid
      return false;
    }
  }

  return current;
}

// Creates a key representing a list value item. For example "box-size:2".
std::string MakeListItemKey(const std::string& name, size_t index) {
  return name + ':' + base::UintToString(index);
}

// Makes a path to widget.app-widget.element.
std::string MakeAppWidgetPath(const std::string& element) {
  return MakeElementPath(keys::kTizenAppWidgetFullKey, element);
}

// Makes a path to widget.app-widget.box-label:index.element.
std::string MakeBoxLabelPath(size_t index, const std::string& element) {
  const std::string& label =
      MakeListItemKey(keys::kTizenAppWidgetBoxLabelKey, index);
  return MakeElementPath(MakeAppWidgetPath(label), element);
}

// Makes a path to widget.app-widget.box-icon.element.
std::string MakeBoxIconPath(const std::string& element) {
  return MakeElementPath(
      MakeAppWidgetPath(keys::kTizenAppWidgetBoxIconKey), element);
}

// Makes a path to widget.app-widget.box-content.element.
std::string MakeBoxContentPath(const std::string& element) {
  return MakeElementPath(
      MakeAppWidgetPath(keys::kTizenAppWidgetBoxContentKey), element);
}

// Makes a path to widget.app-widget.box-content.box-size:index.element.
std::string MakeSizePath(size_t index, const std::string& element) {
  const std::string& size =
      MakeListItemKey(keys::kTizenAppWidgetBoxContentSizeKey, index);
  return MakeElementPath(MakeBoxContentPath(size), element);
}

// Makes a path to widget.app-widget.box-content.pd.element.
std::string MakeDropViewPath(const std::string& element) {
  return MakeElementPath(
      MakeBoxContentPath(keys::kTizenAppWidgetBoxContentDropViewKey), element);
}

}  // namespace

class TizenAppWidgetHandlerTest: public testing::Test {
 public:
  void ChangedOrRemovedFieldTest(const std::string& path,
                                 const std::string* value = nullptr) {
    scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();

    scoped_ptr<base::DictionaryValue> app_widget =
        CreateFullAppWidget("first", "true");

    CHECK(AddDictionary(keys::kTizenAppWidgetFullKey,
        app_widget.Pass(), manifest.get()));

    CHECK(ChangeOrRemove(path, manifest.get(), value));

    scoped_refptr<ApplicationData> application =
        CreateApplication(Manifest::TYPE_WIDGET, *manifest);
    EXPECT_EQ(nullptr, application.get());
  }

  // Defines a test in which a field specified by path is removed from
  // manifest created by CreateDefaultWidgetConfig and complemented by
  // app-widget element created by CreateFullAppWidget.
  // Can be used when path is precise (there is no dictionary list in the way).
  void MissingFieldTest(const std::string& path) {
    ChangedOrRemovedFieldTest(path);
  }

  // Defines a test in which a field specified by path is changed in
  // manifest created by CreateDefaultWidgetConfig and complemented by
  // app-widget element created by CreateFullAppWidget.
  // Can be used when path is precise (there is no dictionary list in the way).
  void InvalidFieldValueTest(const std::string& path,
                             const std::string& value) {
    ChangedOrRemovedFieldTest(path, &value);
  }
};

// Test case for no app-widget element.
TEST_F(TizenAppWidgetHandlerTest, NoAppWidget) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();

  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_WIDGET, *manifest);
  EXPECT_NE(nullptr, application.get());
  EXPECT_EQ(Manifest::TYPE_WIDGET, application->GetManifest()->type());

  const TizenAppWidgetInfo* info = GetInfo(application);
  EXPECT_EQ(nullptr, info);
}

// Test case for one app-widget element with only required attributes and child
// elements set.
TEST_F(TizenAppWidgetHandlerTest, OneMinimalAppWidget) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();

  scoped_ptr<base::DictionaryValue> app_widget =
      CreateMinimalAppWidget("first", "true");

  CHECK(AddDictionary(keys::kTizenAppWidgetFullKey,
      app_widget.Pass(), manifest.get()));

  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_WIDGET, *manifest);
  EXPECT_NE(nullptr, application.get());
  EXPECT_EQ(Manifest::TYPE_WIDGET, application->GetManifest()->type());

  const TizenAppWidgetInfo* info = GetInfo(application);
  EXPECT_NE(nullptr, info);

  const TizenAppWidgetVector& app_widgets = info->app_widgets();
  EXPECT_EQ(1, app_widgets.size());

  const TizenAppWidget& element = app_widgets[0];

  std::vector<std::string> id_parts;
  base::SplitString(element.id, '.', &id_parts);
  EXPECT_EQ(3, id_parts.size());
  EXPECT_EQ("first", id_parts[2]);
  EXPECT_TRUE(element.primary);
  EXPECT_EQ(0, element.update_period.size());
  EXPECT_FALSE(element.auto_launch);
  EXPECT_EQ("sample label", element.label.default_value);
  EXPECT_EQ(0, element.label.lang_value_map.size());
  EXPECT_TRUE(element.icon_src.empty());
  EXPECT_EQ("index.html", element.content_src);
  EXPECT_FALSE(element.content_mouse_event);
  EXPECT_TRUE(element.content_touch_effect);
  EXPECT_EQ(1, element.content_size.size());
  EXPECT_EQ(TizenAppWidgetSizeType::k1x1, element.content_size[0].type);
  EXPECT_TRUE(element.content_size[0].preview.empty());
  EXPECT_TRUE(element.content_size[0].use_decoration);
  EXPECT_EQ(0, element.content_drop_view.size());
}

// Test case for two app-widget elements with only required attributes and child
// elements set.
TEST_F(TizenAppWidgetHandlerTest, TwoMinimalAppWidgets) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();

  scoped_ptr<base::DictionaryValue> app_widget =
      CreateMinimalAppWidget("first", "true");

  CHECK(AddDictionary(keys::kTizenAppWidgetFullKey,
      app_widget.Pass(), manifest.get()));

  app_widget = CreateMinimalAppWidget("second", "false");

  CHECK(AddDictionary(keys::kTizenAppWidgetFullKey,
      app_widget.Pass(), manifest.get()));

  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_WIDGET, *manifest);
  EXPECT_NE(nullptr, application.get());
  EXPECT_EQ(Manifest::TYPE_WIDGET, application->GetManifest()->type());

  const TizenAppWidgetInfo* info = GetInfo(application);
  EXPECT_NE(nullptr, info);

  const TizenAppWidgetVector& app_widgets = info->app_widgets();
  EXPECT_EQ(2, app_widgets.size());

  std::vector<std::string> id_parts;
  base::SplitString(app_widgets[0].id, '.', &id_parts);
  EXPECT_EQ(3, id_parts.size());
  EXPECT_EQ("first", id_parts[2]);
  EXPECT_TRUE(app_widgets[0].primary);

  base::SplitString(app_widgets[1].id, '.', &id_parts);
  EXPECT_EQ(3, id_parts.size());
  EXPECT_EQ("second", id_parts[2]);
  EXPECT_FALSE(app_widgets[1].primary);

  for (const TizenAppWidget& element : app_widgets) {
    EXPECT_EQ(0, element.update_period.size());
    EXPECT_FALSE(element.auto_launch);
    EXPECT_EQ("sample label", element.label.default_value);
    EXPECT_EQ(0, element.label.lang_value_map.size());
    EXPECT_TRUE(element.icon_src.empty());
    EXPECT_EQ("index.html", element.content_src);
    EXPECT_FALSE(element.content_mouse_event);
    EXPECT_TRUE(element.content_touch_effect);
    EXPECT_EQ(1, element.content_size.size());
    EXPECT_EQ(TizenAppWidgetSizeType::k1x1, element.content_size[0].type);
    EXPECT_TRUE(element.content_size[0].preview.empty());
    EXPECT_TRUE(element.content_size[0].use_decoration);
    EXPECT_EQ(0, element.content_drop_view.size());
  }
}

// Test case for one app-widget element with all posible attributes and child
// elements set.
TEST_F(TizenAppWidgetHandlerTest, OneFullAppWidget) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();

  scoped_ptr<base::DictionaryValue> app_widget =
      CreateFullAppWidget("first", "true");

  CHECK(AddDictionary(keys::kTizenAppWidgetFullKey,
      app_widget.Pass(), manifest.get()));

  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_WIDGET, *manifest);
  EXPECT_NE(nullptr, application.get());
  EXPECT_EQ(Manifest::TYPE_WIDGET, application->GetManifest()->type());

  const TizenAppWidgetInfo* info = GetInfo(application);
  EXPECT_NE(nullptr, info);

  const TizenAppWidgetVector& app_widgets = info->app_widgets();
  EXPECT_EQ(1, app_widgets.size());

  const TizenAppWidget& element = app_widgets[0];

  std::vector<std::string> id_parts;
  base::SplitString(element.id, '.', &id_parts);
  EXPECT_EQ(3, id_parts.size());
  EXPECT_EQ("first", id_parts[2]);
  EXPECT_TRUE(element.primary);
  EXPECT_EQ(1, element.update_period.size());
  EXPECT_EQ(1800, element.update_period[0]);
  EXPECT_TRUE(element.auto_launch);
  EXPECT_EQ("sample label", element.label.default_value);
  EXPECT_EQ(1, element.label.lang_value_map.size());
  EXPECT_EQ("sample en label", element.label.lang_value_map.at("en"));
  EXPECT_EQ("icon.png", element.icon_src);
  EXPECT_EQ("index.html", element.content_src);
  EXPECT_TRUE(element.content_mouse_event);
  EXPECT_FALSE(element.content_touch_effect);
  EXPECT_EQ(3, element.content_size.size());
  EXPECT_EQ(TizenAppWidgetSizeType::k1x1, element.content_size[0].type);
  EXPECT_TRUE(element.content_size[0].preview.empty());
  EXPECT_TRUE(element.content_size[0].use_decoration);
  EXPECT_EQ(TizenAppWidgetSizeType::k2x1, element.content_size[1].type);
  EXPECT_EQ("image.png", element.content_size[1].preview);
  EXPECT_TRUE(element.content_size[1].use_decoration);
  EXPECT_EQ(TizenAppWidgetSizeType::k2x2, element.content_size[2].type);
  EXPECT_TRUE(element.content_size[2].preview.empty());
  EXPECT_FALSE(element.content_size[2].use_decoration);
  EXPECT_EQ(1, element.content_drop_view.size());
  EXPECT_EQ("index.html", element.content_drop_view[0].src);
  EXPECT_EQ(720, element.content_drop_view[0].width);
  EXPECT_EQ(150, element.content_drop_view[0].height);
}

// Test case for missing namespace attribute of app-widget element.
TEST_F(TizenAppWidgetHandlerTest, MissingAppWidgetNamespace) {
  MissingFieldTest(MakeAppWidgetPath(keys::kNamespaceKey));
}

// Test case for missing id attribute of app-widget element.
TEST_F(TizenAppWidgetHandlerTest, MissingId) {
  MissingFieldTest(MakeAppWidgetPath(keys::kTizenAppWidgetIdKey));
}

// Test case for missing primary attribute of app-widget element.
TEST_F(TizenAppWidgetHandlerTest, MissingPrimary) {
  MissingFieldTest(MakeAppWidgetPath(keys::kTizenAppWidgetPrimaryKey));
}

// Test case for missing box-label element in app-widget element.
TEST_F(TizenAppWidgetHandlerTest, MissingLabel) {
  MissingFieldTest(MakeAppWidgetPath(keys::kTizenAppWidgetBoxLabelKey));
}

// Test case for missing namespace attribute of box-label element.
TEST_F(TizenAppWidgetHandlerTest, MissingLabelNamespace) {
  MissingFieldTest(MakeBoxLabelPath(0, keys::kNamespaceKey));
}

// Test case for missing namespace attribute of box-icon element.
TEST_F(TizenAppWidgetHandlerTest, MissingIconNamespace) {
  MissingFieldTest(MakeBoxIconPath(keys::kNamespaceKey));
}

// Test case for missing src attribute of box-icon element.
TEST_F(TizenAppWidgetHandlerTest, MissingIconSrc) {
  MissingFieldTest(MakeBoxIconPath(keys::kTizenAppWidgetBoxIconSrcKey));
}

// Test case for missing box-content element in app-widget element.
TEST_F(TizenAppWidgetHandlerTest, MissingContent) {
  MissingFieldTest(MakeAppWidgetPath(keys::kTizenAppWidgetBoxContentKey));
}

// Test case for missing namespace attribute of box-content element.
TEST_F(TizenAppWidgetHandlerTest, MissingContentNamespace) {
  MissingFieldTest(MakeBoxContentPath(keys::kNamespaceKey));
}

// Test case for missing src attribute of box-content element.
TEST_F(TizenAppWidgetHandlerTest, MissingContentSrc) {
  MissingFieldTest(MakeBoxContentPath(keys::kTizenAppWidgetBoxContentSrcKey));
}

// Test case for missing box-size element in box-content element.
TEST_F(TizenAppWidgetHandlerTest, MissingSize) {
  MissingFieldTest(MakeBoxContentPath(keys::kTizenAppWidgetBoxContentSizeKey));
}

// Test case for missing namespace attribute of box-size element.
TEST_F(TizenAppWidgetHandlerTest, MissingSizeNamespace) {
  MissingFieldTest(MakeSizePath(0, keys::kNamespaceKey));
}

// Test case for missing box-size element with 1x1 value in box-content element.
TEST_F(TizenAppWidgetHandlerTest, MissingSize1x1) {
  MissingFieldTest(MakeBoxContentPath(MakeListItemKey(
      keys::kTizenAppWidgetBoxContentSizeKey, 0)));
}

// Test case for missing namespace attribute of pd element.
TEST_F(TizenAppWidgetHandlerTest, MissingDropViewNamespace) {
  MissingFieldTest(MakeDropViewPath(keys::kNamespaceKey));
}

// Test case for missing src attribute of pd element.
TEST_F(TizenAppWidgetHandlerTest, MissingDropViewSrc) {
  MissingFieldTest(MakeDropViewPath(
      keys::kTizenAppWidgetBoxContentDropViewSrcKey));
}

// Test case for missing width attribute of pd element.
TEST_F(TizenAppWidgetHandlerTest, MissingWidth) {
  MissingFieldTest(MakeDropViewPath(
      keys::kTizenAppWidgetBoxContentDropViewWidthKey));
}

// Test case for missing height attribute of pd element.
TEST_F(TizenAppWidgetHandlerTest, MissingHeight) {
  MissingFieldTest(MakeDropViewPath(
      keys::kTizenAppWidgetBoxContentDropViewHeightKey));
}

// Test case for invalid namespace attribute of app-widget element.
TEST_F(TizenAppWidgetHandlerTest, InvalidAppWidgetNamespace) {
  InvalidFieldValueTest(
      MakeAppWidgetPath(keys::kNamespaceKey),
      "not_a_tizen_namespace");
}

// Test case for invalid id attribute of app-widget element.
// The id must start with application id.
TEST_F(TizenAppWidgetHandlerTest, InvalidIdNotStartedWithAppId) {
  InvalidFieldValueTest(
      MakeAppWidgetPath(keys::kTizenAppWidgetIdKey),
      "invalid_id");
}

// Test case for invalid id attribute of app-widget element.
// The id must consist of 0-9a-zA-Z.
TEST_F(TizenAppWidgetHandlerTest, InvalidIdInvalidCharInNamePart) {
  InvalidFieldValueTest(
      MakeAppWidgetPath(keys::kTizenAppWidgetIdKey),
      CreateAppWidgetId("the-first"));
}

// Test case for invalid primary attribute of app-widget element.
// The value must be bool (true or false).
TEST_F(TizenAppWidgetHandlerTest, InvalidPrimary) {
  InvalidFieldValueTest(
      MakeAppWidgetPath(keys::kTizenAppWidgetPrimaryKey),
      "not_true_nor_false");
}

// Test case for invalid update-period attribute of app-widget element.
// The value must be integer.
TEST_F(TizenAppWidgetHandlerTest, InvalidUpdatePeriod) {
  InvalidFieldValueTest(
      MakeAppWidgetPath(keys::kTizenAppWidgetUpdatePeriodKey),
      "not_a_number");
}

// Test case for invalid update-period attribute of app-widget element.
// The value must be not less than 1800.
TEST_F(TizenAppWidgetHandlerTest, InvalidUpdatePeriodOutOfDomain) {
  InvalidFieldValueTest(
      MakeAppWidgetPath(keys::kTizenAppWidgetUpdatePeriodKey),
      "1799");
}

// Test case for invalid auto-launch attribute of app-widget element.
// The value must be bool (true or false).
TEST_F(TizenAppWidgetHandlerTest, InvalidAutoLaunch) {
  InvalidFieldValueTest(
      MakeAppWidgetPath(keys::kTizenAppWidgetAutoLaunchKey),
      "not_true_nor_false");
}

// Test case for invalid namespace attribute of box-label element.
TEST_F(TizenAppWidgetHandlerTest, InvalidLabelNamespace) {
  InvalidFieldValueTest(
      MakeBoxLabelPath(0, keys::kNamespaceKey),
      "not_a_tizen_namespace");
}

// Test case for invalid namespace attribute of box-icon element.
TEST_F(TizenAppWidgetHandlerTest, InvalidIconNamespace) {
  InvalidFieldValueTest(
      MakeBoxIconPath(keys::kNamespaceKey),
      "not_a_tizen_namespace");
}

// Test case for invalid namespace attribute of box-content element.
TEST_F(TizenAppWidgetHandlerTest, InvalidContentNamespace) {
  InvalidFieldValueTest(
      MakeBoxContentPath(keys::kNamespaceKey),
      "not_a_tizen_namespace");
}

// Test case for invalid mouse-event attribute of box-content element.
// The value must be bool (true or false).
TEST_F(TizenAppWidgetHandlerTest, InvalidMouseEvent) {
  InvalidFieldValueTest(
      MakeBoxContentPath(keys::kTizenAppWidgetBoxContentMouseEventKey),
      "not_true_nor_false");
}

// Test case for invalid touch-effect attribute of box-content element.
// The value must be bool (true or false).
TEST_F(TizenAppWidgetHandlerTest, InvalidTouchEffect) {
  InvalidFieldValueTest(
      MakeBoxContentPath(keys::kTizenAppWidgetBoxContentTouchEffectKey),
      "not_true_nor_false");
}

// Test case for invalid namespace attribute of box-size element.
TEST_F(TizenAppWidgetHandlerTest, InvalidSizeNamespace) {
  InvalidFieldValueTest(
      MakeSizePath(0, keys::kNamespaceKey),
      "not_a_tizen_namespace");
}

// Test case for invalid text value of box-size element.
// The value must be one of these: 1x1, 2x1, 2x2.
TEST_F(TizenAppWidgetHandlerTest, InvalidSize) {
  InvalidFieldValueTest(
      MakeSizePath(0, keys::kTizenAppWidgetBoxContentSizeTextKey),
      "AxB");
}

// Test case for invalid use-decoration attribute of box-size element.
// The value must be bool (true or false).
TEST_F(TizenAppWidgetHandlerTest, InvalidUseDecoration) {
  InvalidFieldValueTest(
      MakeSizePath(2, keys::kTizenAppWidgetBoxContentSizeUseDecorationKey),
      "not_true_nor_false");
}

// Test case for invalid namespace attribute of pd element.
TEST_F(TizenAppWidgetHandlerTest, InvalidDropViewNamespace) {
  InvalidFieldValueTest(
      MakeDropViewPath(keys::kNamespaceKey),
      "not_a_tizen_namespace");
}

// Test case for invalid width attribute of pd element.
// The value must be integer.
TEST_F(TizenAppWidgetHandlerTest, InvalidWidth) {
  InvalidFieldValueTest(
      MakeDropViewPath(keys::kTizenAppWidgetBoxContentDropViewWidthKey),
      "not_a_number");
}

// Test case for invalid height attribute of pd element.
// The value must be integer.
TEST_F(TizenAppWidgetHandlerTest, InvalidHeight) {
  InvalidFieldValueTest(
      MakeDropViewPath(keys::kTizenAppWidgetBoxContentDropViewHeightKey),
      "not_a_number");
}

// Test case for invalid height attribute of pd element.
// The value must be in a range of [1, 380].
TEST_F(TizenAppWidgetHandlerTest, InvalidHeightOutOfDomain) {
  InvalidFieldValueTest(
      MakeDropViewPath(keys::kTizenAppWidgetBoxContentDropViewHeightKey),
      "381");
}

// Test case for more than one box-content element.
TEST_F(TizenAppWidgetHandlerTest, TwoContents) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();

  scoped_ptr<base::DictionaryValue> app_widget =
      CreateFullAppWidget("first", "true");

  scoped_ptr<base::DictionaryValue> box_content =
      CreateBoxContent("index.html");

  CHECK(AddDictionary(keys::kTizenAppWidgetBoxContentSizeKey,
      CreateBoxSize("1x1"), box_content.get()));

  CHECK(AddDictionary(keys::kTizenAppWidgetBoxContentKey,
      box_content.Pass(), app_widget.get()));

  CHECK(AddDictionary(keys::kTizenAppWidgetFullKey,
      app_widget.Pass(), manifest.get()));

  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_WIDGET, *manifest);
  EXPECT_EQ(nullptr, application.get());
}

// Test case for more than one box-icon element.
TEST_F(TizenAppWidgetHandlerTest, TwoIcons) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();

  scoped_ptr<base::DictionaryValue> app_widget =
      CreateFullAppWidget("first", "true");

  CHECK(AddDictionary(keys::kTizenAppWidgetBoxIconKey,
      CreateBoxIcon("icon.png"), app_widget.get()));

  CHECK(AddDictionary(keys::kTizenAppWidgetFullKey,
      app_widget.Pass(), manifest.get()));

  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_WIDGET, *manifest);
  EXPECT_EQ(nullptr, application.get());
}

// Test case for more than one pd element.
TEST_F(TizenAppWidgetHandlerTest, TwoDropViews) {
  scoped_ptr<base::DictionaryValue> manifest = CreateDefaultWidgetConfig();

  scoped_ptr<base::DictionaryValue> app_widget =
      CreateFullAppWidget("first", "true");

  CHECK(AddDictionary(
      MakeElementPath(keys::kTizenAppWidgetBoxContentKey,
          keys::kTizenAppWidgetBoxContentDropViewKey),
      CreateBoxDropView("index.html", "720", "150"), app_widget.get()));

  CHECK(AddDictionary(keys::kTizenAppWidgetFullKey,
      app_widget.Pass(), manifest.get()));

  scoped_refptr<ApplicationData> application =
      CreateApplication(Manifest::TYPE_WIDGET, *manifest);
  EXPECT_EQ(nullptr, application.get());
}

}  // namespace application
}  // namespace xwalk
