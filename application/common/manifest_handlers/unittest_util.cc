// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/unittest_util.h"

#include "base/strings/string_util.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace manifest_keys = application_manifest_keys;
namespace widget_keys = application_widget_keys;

namespace application {

namespace {

// Makes a path to widget.element.
std::string MakeWidgetPath(const std::string& element) {
  return MakeElementPath(widget_keys::kWidgetKey, element);
}

#if defined(OS_TIZEN)

// Makes a path to widget.application.element.
std::string MakeApplicationPath(const std::string& element) {
  return MakeElementPath(widget_keys::kTizenApplicationKey, element);
}

// Creates an app-widget element id basing on values of default package id
// and default application name.
std::string GetDefaultApplicationId() {
  std::vector<std::string> parts;
  parts.push_back(kDefaultWidgetPackageId);
  parts.push_back(kDefaultWidgetApplicationName);
  return JoinString(parts, '.');
}

#endif  // defined(OS_TIZEN)

}  // namespace

const char kDefaultManifestName[] = "no name";
const char kDefaultManifestVersion[] = "0";
const char kDefaultWidgetName[] = "no name";
const char kDefaultWidgetVersion[] = "0";

#if defined(OS_TIZEN)

const char kDefaultWidgetPackageId[] = "abcdefghij";
const char kDefaultWidgetApplicationName[] = "noname";
const char kDefaultWidgetRequiredVersion[] = "0";

#endif  // defined(OS_TIZEN)

scoped_ptr<base::DictionaryValue> CreateDefaultManifestConfig() {
  scoped_ptr<base::DictionaryValue> manifest(new base::DictionaryValue());

  manifest->SetString(manifest_keys::kXWalkVersionKey, kDefaultManifestVersion);
  manifest->SetString(manifest_keys::kNameKey, kDefaultManifestName);

  return manifest.Pass();
}

scoped_ptr<base::DictionaryValue> CreateDefaultWidgetConfig() {
  scoped_ptr<base::DictionaryValue> manifest(new base::DictionaryValue());

  // widget attributes

  manifest->SetString(MakeWidgetPath(widget_keys::kNamespaceKey),
                      widget_keys::kWidgetNamespacePrefix);
  manifest->SetString(widget_keys::kVersionKey, kDefaultWidgetVersion);
  manifest->SetString(widget_keys::kNameKey, kDefaultWidgetName);

#if defined(OS_TIZEN)

  // widget.application attributes

  manifest->SetString(
      MakeApplicationPath(widget_keys::kNamespaceKey),
      widget_keys::kTizenNamespacePrefix);
  manifest->SetString(
      MakeApplicationPath(widget_keys::kTizenApplicationIdKey),
      GetDefaultApplicationId());
  manifest->SetString(
      MakeApplicationPath(widget_keys::kTizenApplicationPackageKey),
      kDefaultWidgetPackageId);
  manifest->SetString(
      MakeApplicationPath(widget_keys::kTizenApplicationRequiredVersionKey),
      kDefaultWidgetRequiredVersion);

#endif

  return manifest.Pass();
}

scoped_refptr<ApplicationData> CreateApplication(Manifest::Type type,
    const base::DictionaryValue& manifest) {
  std::string error;
  scoped_refptr<ApplicationData> application = ApplicationData::Create(
      base::FilePath(), std::string(), ApplicationData::LOCAL_DIRECTORY,
      make_scoped_ptr(new Manifest(make_scoped_ptr(manifest.DeepCopy()), type)),
      &error);
  return application;
}

std::string MakeElementPath(const std::string& parent,
    const std::string& element) {
  std::vector<std::string> parts;
  parts.push_back(parent);
  parts.push_back(element);
  return JoinString(parts, '.');
}

bool AddDictionary(const std::string& key,
    scoped_ptr<base::DictionaryValue> child, base::DictionaryValue* parent) {
  if (key.empty() || !child || !parent)
    return false;

  scoped_ptr<base::Value> existing_child;
  base::DictionaryValue* unused;
  if (parent->GetDictionary(key, &unused)) {
    if (!parent->Remove(key, &existing_child))
      return false;
  }

  if (existing_child) {
    scoped_ptr<base::ListValue> list(new base::ListValue);
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
