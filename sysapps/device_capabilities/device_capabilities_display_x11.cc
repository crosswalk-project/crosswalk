// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/device_capabilities_display.h"

#include <X11/Xlib.h>

#include <sstream>

#include "base/logging.h"

namespace xwalk {
namespace sysapps {

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
  NOTIMPLEMENTED() << "Tizen doesn't support multi-display";
}

void DeviceCapabilitiesDisplay::RemoveEventListener(
    DeviceCapabilitiesInstance* instance) {
  NOTIMPLEMENTED();
}

void DeviceCapabilitiesDisplay::QueryDisplayUnits() {
    // Currently, Tizen doesn't support multi-display,
    // So, here we only show the default display info.
    Display* dpy = XOpenDisplay(NULL);
    if (NULL == dpy) {
      LOG(ERROR) << "XOpenDisplay failed";
    }

    DeviceDisplayUnit unit;
    unit.id = 0;
    // FIXME(YuZhiqiangX): find which field reflects 'name'
    unit.name = "";
    unit.isPrimary = true;
    unit.isInternal = true;
    unit.width = DisplayWidth(dpy, DefaultScreen(dpy));;
    unit.height = DisplayHeight(dpy, DefaultScreen(dpy));;
    int physicalwidth = DisplayWidthMM(dpy, DefaultScreen(dpy));;
    int physicalheight = DisplayHeightMM(dpy, DefaultScreen(dpy));
    if (physicalwidth * physicalheight != 0) {
      unit.dpiX = static_cast<int>((unit.width * 25.4) / physicalwidth);
      unit.dpiY = static_cast<int>((unit.height * 25.4) / physicalheight);
    }
    // FIXME(YuZhiqiangX): find which field reflects 'availWidth'
    unit.availWidth = unit.width;
    // FIXME(YuZhiqiangX): find which field reflects 'availHeight'
    unit.availHeight = unit.height;

    displays_[unit.id] = unit;
    XCloseDisplay(dpy);
  return;
}

void DeviceCapabilitiesDisplay::SetJsonValue(Json::Value* obj,
                                             const DeviceDisplayUnit& unit) {
  std::stringstream id_string;
  id_string << unit.id;
  (*obj)["id"] = Json::Value(id_string.str());
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
