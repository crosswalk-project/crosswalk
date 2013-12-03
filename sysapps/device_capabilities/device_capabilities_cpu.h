// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_CPU_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_CPU_H_

#include <string>

#include "third_party/jsoncpp/source/include/json/json.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_utils.h"

namespace xwalk {
namespace sysapps {

class DeviceCapabilitiesCpu : public DeviceCapabilitiesObject {
 public:
  static DeviceCapabilitiesObject& GetDeviceInstance() {
    static DeviceCapabilitiesCpu instance;
    return instance;
  }
  Json::Value* Get();
  void AddEventListener(const std::string& event_name,
                        DeviceCapabilitiesInstance* instance) { }
  void RemoveEventListener(DeviceCapabilitiesInstance* instance) { }

 private:
  explicit DeviceCapabilitiesCpu();

  bool QueryLoad();
  void SetJsonValue(Json::Value* obj);

  int numOfProcessors_;
  std::string archName_;
  double load_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_CPU_H_
