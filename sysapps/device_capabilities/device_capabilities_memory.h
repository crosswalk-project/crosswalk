// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_MEMORY_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_MEMORY_H_

#include <string>

#include "third_party/jsoncpp/source/include/json/json.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_utils.h"

namespace xwalk {
namespace sysapps {

class DeviceCapabilitiesMemory : public DeviceCapabilitiesObject {
 public:
  static DeviceCapabilitiesObject& GetDeviceInstance() {
    static DeviceCapabilitiesMemory instance;
    return instance;
  }
  Json::Value* Get();
  void AddEventListener(const std::string& event_name,
                        DeviceCapabilitiesInstance* instance) { }
  void RemoveEventListener(DeviceCapabilitiesInstance* instance) { }

 private:
  explicit DeviceCapabilitiesMemory()
      : capacity_(0),
        availCapacity_(0) { }

  bool QueryCapacity();
  bool QueryAvailableCapacity();
  void SetJsonValue(Json::Value* obj);

  unsigned int capacity_;
  unsigned int availCapacity_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_MEMORY_H_
