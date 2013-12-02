// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/device_capabilities_display.h"

#include <sstream>
#include <vector>

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "ui/gfx/display.h"
#include "ui/gfx/screen.h"

namespace xwalk {
namespace sysapps {

// Currently we follow the way Chromium calculate DPI for ChromeOS:
// reference to 'display_info_provider_chromeos.cc' in
// 'chrome/browser/extensions/api/system_display/'
// FIXME(YuZhiqiangX): determine the DPI of the screen.
const float kDpi96 = 96;

DeviceCapabilitiesDisplay::DeviceCapabilitiesDisplay() {
  QueryDisplayUnits();
}

Json::Value* DeviceCapabilitiesDisplay::Get() {
  Json::Value* obj = new Json::Value();
  Json::Value displays;

  for (DisplaysMap::iterator it = displays_.begin();
       it != displays_.end(); it++) {
    Json::Value unit;
    SetJsonValue(&unit, it->second);
    displays.append(unit);
  }

  (*obj)["displays"] = displays;
  return obj;
}

void DeviceCapabilitiesDisplay::AddEventListener(const std::string& event_name,
    DeviceCapabilitiesInstance* instance) {
  NOTIMPLEMENTED();
}

void DeviceCapabilitiesDisplay::RemoveEventListener(
    DeviceCapabilitiesInstance* instance) {
  NOTIMPLEMENTED();
}

void DeviceCapabilitiesDisplay::QueryDisplayUnits() {
  gfx::Screen* screen = gfx::Screen::GetNativeScreen();
  int64 primary_id = screen->GetPrimaryDisplay().id();
  std::vector<gfx::Display> displays = screen->GetAllDisplays();

  for (int i = 0; i < screen->GetNumDisplays(); ++i) {
    DeviceDisplayUnit unit;
    unit.id = displays[i].id();
    // FIXME(YuZhiqiangX): find which field reflects 'name'.
    unit.name = "";
    unit.isPrimary = (displays[i].id() == primary_id);
    unit.isInternal = displays[i].IsInternal();
    unit.width = displays[i].bounds().width();
    unit.height = displays[i].bounds().height();
    const float dpi = displays[i].device_scale_factor() * kDpi96;
    unit.dpiX = static_cast<unsigned int>(dpi);
    unit.dpiY = static_cast<unsigned int>(dpi);
    unit.availWidth = displays[i].work_area_size().width();
    unit.availHeight = displays[i].work_area_size().height();
    displays_[unit.id] = unit;
  }

  return;
}

void DeviceCapabilitiesDisplay::SetJsonValue(Json::Value* obj,
                                             const DeviceDisplayUnit& unit) {
  (*obj)["id"] = Json::Value(base::Uint64ToString(unit.id));
  (*obj)["name"] = Json::Value(unit.name);
  (*obj)["isPrimary"] = Json::Value(unit.isPrimary);
  (*obj)["isInternal"] = Json::Value(unit.isInternal);
  (*obj)["dpiX"] = Json::Value(static_cast<double>(unit.dpiX));
  (*obj)["dpiY"] = Json::Value(static_cast<double>(unit.dpiY));
  (*obj)["width"] = Json::Value(static_cast<double>(unit.width));
  (*obj)["height"] = Json::Value(static_cast<double>(unit.height));
  (*obj)["availWidth"] = Json::Value(static_cast<double>(unit.availWidth));
  (*obj)["availHeight"] = Json::Value(static_cast<double>(unit.availHeight));
}

}  // namespace sysapps
}  // namespace xwalk
