// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/unittest_util.h"

#include <string>
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest.h"

namespace xwalk {

namespace keys = application_widget_keys;

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

scoped_ptr<base::DictionaryValue> CreateDefaultW3CManifest() {
  scoped_ptr<base::DictionaryValue> manifest(new base::DictionaryValue());

  // widget attributes

  manifest->SetString(
      DotConnect(keys::kWidgetKey, keys::kNamespaceKey),
      kW3CNamespacePrefix);

  manifest->SetString(keys::kVersionKey, kDefaultVersion);

  manifest->SetString(keys::kNameKey, kDefaultName);

  return manifest.Pass();
}

#if defined(OS_TIZEN)

scoped_ptr<base::DictionaryValue> CreateDefaultWGTManifest() {
  scoped_ptr<base::DictionaryValue> manifest(CreateDefaultW3CManifest());

  // widget.application attributes

  manifest->SetString(
      DotConnect(keys::kTizenApplicationKey,
                 keys::kNamespaceKey),
      kTizenNamespacePrefix);

  manifest->SetString(
      DotConnect(keys::kTizenApplicationKey,
                 keys::kTizenApplicationIdKey),
      DotConnect(kDefaultPackageId, kDefaultApplicationName));

  manifest->SetString(
      DotConnect(keys::kTizenApplicationKey,
                 keys::kTizenApplicationPackageKey),
      kDefaultPackageId);

  manifest->SetString(
      DotConnect(keys::kTizenApplicationKey,
                 keys::kTizenApplicationRequiredVersionKey),
      kDefaultRequiredVersion);

  return manifest.Pass();
}

#endif  // defined(OS_TIZEN)

scoped_refptr<ApplicationData> CreateApplication(
    const base::DictionaryValue& manifest) {
  std::string error;
  scoped_refptr<ApplicationData> application = ApplicationData::Create(
      base::FilePath(), std::string(), ApplicationData::LOCAL_DIRECTORY,
      make_scoped_ptr(new Manifest(make_scoped_ptr(manifest.DeepCopy()),
                                   Manifest::TYPE_WIDGET)),
      &error);
  return application;
}

}  // namespace application
}  // namespace xwalk
