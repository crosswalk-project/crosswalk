// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/unittest_util.h"

#include <string>
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace manifest_keys = application_manifest_keys;
namespace widget_keys = application_widget_keys;

namespace application {

namespace {

const char kDefaultVersion[] = "0";
const char kDefaultName[] = "no name";

#if defined(OS_TIZEN)

const char kDefaultPackageId[] = "abcdefghij";
const char kDefaultApplicationName[] = "noname";
const char kDefaultRequiredVersion[] = "0";

#endif  // defined(OS_TIZEN)

std::string DotConnect(const std::string& first, const std::string& second) {
  return first + '.' + second;
}

}  // namespace

scoped_ptr<base::DictionaryValue> CreateDefaultManifestConfig() {
  scoped_ptr<base::DictionaryValue> manifest(new base::DictionaryValue());

  manifest->SetString(manifest_keys::kXWalkVersionKey, kDefaultVersion);
  manifest->SetString(manifest_keys::kNameKey, kDefaultName);

  return manifest.Pass();
}

scoped_ptr<base::DictionaryValue> CreateDefaultWidgetConfig() {
  scoped_ptr<base::DictionaryValue> manifest(new base::DictionaryValue());

  // widget attributes

  manifest->SetString(
      DotConnect(widget_keys::kWidgetKey, widget_keys::kNamespaceKey),
      widget_keys::kWidgetNamespacePrefix);
  manifest->SetString(widget_keys::kVersionKey, kDefaultVersion);
  manifest->SetString(widget_keys::kNameKey, kDefaultName);

#if defined(OS_TIZEN)

  // widget.application attributes

  manifest->SetString(
      DotConnect(widget_keys::kTizenApplicationKey,
                 widget_keys::kNamespaceKey),
      widget_keys::kTizenNamespacePrefix);
  manifest->SetString(
      DotConnect(widget_keys::kTizenApplicationKey,
                 widget_keys::kTizenApplicationIdKey),
      DotConnect(kDefaultPackageId, kDefaultApplicationName));
  manifest->SetString(
      DotConnect(widget_keys::kTizenApplicationKey,
                 widget_keys::kTizenApplicationPackageKey),
      kDefaultPackageId);
  manifest->SetString(
      DotConnect(widget_keys::kTizenApplicationKey,
                 widget_keys::kTizenApplicationRequiredVersionKey),
      kDefaultRequiredVersion);

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

}  // namespace application
}  // namespace xwalk
