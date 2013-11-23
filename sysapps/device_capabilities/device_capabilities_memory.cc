// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/device_capabilities_memory.h"

#include "base/sys_info.h"

namespace xwalk {
namespace sysapps {

Json::Value* DeviceCapabilitiesMemory::Get() {
  Json::Value* obj = new Json::Value();
  if (QueryCapacity() && QueryAvailableCapacity()) {
    SetJsonValue(obj);
    return obj;
  }
  (*obj)["error"] = Json::Value("Get MemoryInfo failed");
  return obj;
}

void DeviceCapabilitiesMemory::SetJsonValue(Json::Value* obj) {
  (*obj)["capacity"] = Json::Value(static_cast<double>(capacity_));
  (*obj)["availCapacity"] = Json::Value(
      static_cast<double>(availCapacity_));
}

bool DeviceCapabilitiesMemory::QueryCapacity() {
  capacity_ = base::SysInfo::AmountOfPhysicalMemory();
  return true;
}

bool DeviceCapabilitiesMemory::QueryAvailableCapacity() {
  availCapacity_ = base::SysInfo::AmountOfAvailablePhysicalMemory();
  return true;
}

}  // namespace sysapps
}  // namespace xwalk
