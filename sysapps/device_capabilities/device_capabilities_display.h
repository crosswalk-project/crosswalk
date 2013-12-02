// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_DISPLAY_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_DISPLAY_H_

#include <map>
#include <string>

#include "third_party/jsoncpp/source/include/json/json.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_utils.h"

namespace xwalk {
namespace sysapps {

struct DeviceDisplayUnit {
  int64 id;
  std::string name;
  bool isPrimary;
  bool isInternal;
  unsigned int dpiX;
  unsigned int dpiY;
  double width;
  double height;
  double availWidth;
  double availHeight;
};

class DeviceCapabilitiesDisplay : public DeviceCapabilitiesObject {
 public:
  static DeviceCapabilitiesObject& GetDeviceInstance() {
    static DeviceCapabilitiesDisplay instance;
    return instance;
  }
  Json::Value* Get();
  void AddEventListener(const std::string& event_name,
                        DeviceCapabilitiesInstance* instance);
  void RemoveEventListener(DeviceCapabilitiesInstance* instance);

 private:
  explicit DeviceCapabilitiesDisplay();

  void SetJsonValue(Json::Value* obj, const DeviceDisplayUnit& unit);
  void QueryDisplayUnits();

  typedef std::map<int64, DeviceDisplayUnit> DisplaysMap;
  DisplaysMap displays_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_DISPLAY_H_
