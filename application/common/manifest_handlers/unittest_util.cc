// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/unittest_util.h"

#include "base/memory/ptr_util.h"
#include "base/strings/string_util.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/id_util.h"

namespace xwalk {

namespace manifest_keys = application_manifest_keys;
namespace widget_keys = application_widget_keys;

namespace application {

namespace {

// Makes a path to widget.element.
std::string MakeWidgetPath(const std::string& element) {
  return MakeElementPath(widget_keys::kWidgetKey, element);
}

}  // namespace

const char kDefaultManifestName[] = "no name";
const char kDefaultManifestVersion[] = "0";
const char kDefaultWidgetName[] = "no name";
const char kDefaultWidgetVersion[] = "0";

std::unique_ptr<base::DictionaryValue> CreateDefaultManifestConfig() {
  std::unique_ptr<base::DictionaryValue> manifest(new base::DictionaryValue());

  manifest->SetString(manifest_keys::kXWalkVersionKey, kDefaultManifestVersion);
  manifest->SetString(manifest_keys::kNameKey, kDefaultManifestName);

  return manifest;
}

std::unique_ptr<base::DictionaryValue> CreateDefaultWidgetConfig() {
  std::unique_ptr<base::DictionaryValue> manifest(new base::DictionaryValue());

  // widget attributes

  manifest->SetString(MakeWidgetPath(widget_keys::kNamespaceKey),
                      widget_keys::kWidgetNamespacePrefix);
  manifest->SetString(widget_keys::kVersionKey, kDefaultWidgetVersion);
  manifest->SetString(widget_keys::kNameKey, kDefaultWidgetName);
  return manifest;
}

scoped_refptr<ApplicationData> CreateApplication(Manifest::Type type,
    const base::DictionaryValue& manifest) {
  std::string error;
  scoped_refptr<ApplicationData> application = ApplicationData::Create(
      base::FilePath(), GenerateId("test"), ApplicationData::LOCAL_DIRECTORY,
      base::WrapUnique(new Manifest(base::WrapUnique(manifest.DeepCopy()), type)),
      &error);
  EXPECT_TRUE(error.empty()) << error;
  return application;
}

std::string MakeElementPath(const std::string& parent,
    const std::string& element) {
  std::vector<std::string> parts;
  parts.push_back(parent);
  parts.push_back(element);
  return base::JoinString(parts, ".");
}

bool AddDictionary(const std::string& key,
    std::unique_ptr<base::DictionaryValue> child, base::DictionaryValue* parent) {
  if (key.empty() || !child || !parent)
    return false;

  std::unique_ptr<base::Value> existing_child;
  base::DictionaryValue* unused;
  if (parent->GetDictionary(key, &unused)) {
    if (!parent->Remove(key, &existing_child))
      return false;
  }

  if (existing_child) {
    std::unique_ptr<base::ListValue> list(new base::ListValue);
    list->Set(list->GetSize(), existing_child.release());
    list->Set(list->GetSize(), child.release());
    parent->Set(key, list.release());
  } else {
    base::ListValue* list;
    if (parent->GetList(key, &list))
      list->Set(list->GetSize(), child.release());
    else
      parent->Set(key, child.release());
  }

  return true;
}

}  // namespace application
}  // namespace xwalk
