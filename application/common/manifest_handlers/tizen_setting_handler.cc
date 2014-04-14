// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/tizen_setting_handler.h"

#include <map>
#include <utility>

#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

TizenSettingInfo::TizenSettingInfo()
    : hwkey_enabled_(true) {}

TizenSettingInfo::~TizenSettingInfo() {}

TizenSettingHandler::TizenSettingHandler() {}

TizenSettingHandler::~TizenSettingHandler() {}

bool TizenSettingHandler::Parse(scoped_refptr<ApplicationData> application,
                                base::string16* error) {
  scoped_ptr<TizenSettingInfo> app_info(new TizenSettingInfo);
  const Manifest* manifest = application->GetManifest();
  DCHECK(manifest);

  std::string hwkey;
  manifest->GetString(keys::kTizenHardwareKey, &hwkey);
  app_info->set_hwkey_enabled(hwkey != "disable");

  application->SetManifestData(keys::kTizenSettingKey,
                               app_info.release());
  return true;
}

bool TizenSettingHandler::Validate(
    scoped_refptr<const ApplicationData> application,
    std::string* error,
    std::vector<InstallWarning>* warnings) const {
  const Manifest* manifest = application->GetManifest();
  DCHECK(manifest);
  std::string hwkey;
  manifest->GetString(keys::kTizenHardwareKey, &hwkey);
  if (!hwkey.empty() && hwkey != "enable" && hwkey != "disable") {
    *error = std::string("The hwkey value must be 'enable'/'disable',"
                         " or not specified in configuration file.");
    return false;
  }
  return true;
}

std::vector<std::string> TizenSettingHandler::Keys() const {
  return std::vector<std::string>(1, keys::kTizenSettingKey);
}

}  // namespace application
}  // namespace xwalk
