// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/tizen_setting_handler.h"

#include <map>
#include <utility>

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

TizenSettingInfo::TizenSettingInfo()
    : hwkey_enabled_(true),
      screen_orientation_(PORTRAIT),
      background_support_enabled_(false) {}

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

  std::string screen_orientation;
  manifest->GetString(keys::kTizenScreenOrientationKey, &screen_orientation);
  if (base::strcasecmp("portrait", screen_orientation.c_str()) == 0)
    app_info->set_screen_orientation(TizenSettingInfo::PORTRAIT);
  else if (base::strcasecmp("landscape", screen_orientation.c_str()) == 0)
    app_info->set_screen_orientation(TizenSettingInfo::LANDSCAPE);
  else
    app_info->set_screen_orientation(TizenSettingInfo::AUTO);
  std::string encryption;
  manifest->GetString(keys::kTizenEncryptionKey, &encryption);
  app_info->set_encryption_enabled(encryption == "enable");

  std::string context_menu;
  manifest->GetString(keys::kTizenContextMenuKey, &context_menu);
  app_info->set_context_menu_enabled(context_menu != "disable");

  std::string background_support;
  manifest->GetString(keys::kTizenBackgroundSupportKey, &background_support);
  app_info->set_background_support_enabled(background_support == "enable");

  application->SetManifestData(keys::kTizenSettingKey,
                               app_info.release());
  return true;
}

bool TizenSettingHandler::Validate(
    scoped_refptr<const ApplicationData> application,
    std::string* error) const {
  const Manifest* manifest = application->GetManifest();
  DCHECK(manifest);
  std::string hwkey;
  manifest->GetString(keys::kTizenHardwareKey, &hwkey);
  if (!hwkey.empty() && hwkey != "enable" && hwkey != "disable") {
    *error = std::string("The hwkey value must be 'enable'/'disable',"
                         " or not specified in configuration file.");
    return false;
  }

  std::string screen_orientation;
  manifest->GetString(keys::kTizenScreenOrientationKey, &screen_orientation);
  if (!screen_orientation.empty() &&
      base::strcasecmp("portrait", screen_orientation.c_str()) != 0 &&
      base::strcasecmp("landscape", screen_orientation.c_str()) != 0 &&
      base::strcasecmp("auto-rotation", screen_orientation.c_str()) != 0) {
    *error = std::string("The screen-orientation must be 'portrait'/"
                         "'landscape'/'auto-rotation' or not specified.");
    return false;
  }
  std::string encryption;
  manifest->GetString(keys::kTizenEncryptionKey, &encryption);
  if (!encryption.empty() && encryption != "enable" &&
      encryption != "disable") {
    *error = std::string("The encryption value must be 'enable'/'disable', "
                         "or not specified in configuration file.");
    return false;
  }
  std::string context_menu;
  manifest->GetString(keys::kTizenContextMenuKey, &context_menu);
  if (!context_menu.empty() &&
      context_menu != "enable" &&
      context_menu != "disable") {
    *error = std::string("The context-menu value must be 'enable'/'disable', "
                         "or not specified in configuration file.");
    return false;
  }
  std::string background_support;
  manifest->GetString(keys::kTizenBackgroundSupportKey, &background_support);
  if (!background_support.empty() &&
      background_support != "enable" &&
      background_support != "disable") {
    *error = std::string("The background-support value must be"
                         "'enable'/'disable', or not specified in configuration"
                         "file.");
  }
  return true;
}

std::vector<std::string> TizenSettingHandler::Keys() const {
  return std::vector<std::string>(1, keys::kTizenSettingKey);
}

}  // namespace application
}  // namespace xwalk
