// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/device_capabilities_cpu.h"

#include "base/logging.h"
#include "base/sys_info.h"

namespace xwalk {
namespace sysapps {

DeviceCapabilitiesCpu::DeviceCapabilitiesCpu()
    : numOfProcessors_(0),
      archName_("Unknown"),
      load_(0.0) {
  numOfProcessors_ = base::SysInfo::NumberOfProcessors();
  archName_ = base::SysInfo::OperatingSystemArchitecture();
}

Json::Value* DeviceCapabilitiesCpu::Get() {
  Json::Value* obj = new Json::Value();
  if (QueryLoad()) {
    SetJsonValue(obj);
    return obj;
  }
  (*obj)["error"] = Json::Value("Get CpuInfo failed");
  return obj;
}

void DeviceCapabilitiesCpu::SetJsonValue(Json::Value* obj) {
  (*obj)["numOfProcessors"] = Json::Value(
      static_cast<double>(numOfProcessors_));
  (*obj)["archName"] = Json::Value(archName_);
  (*obj)["load"] = Json::Value(load_);
}

bool DeviceCapabilitiesCpu::QueryLoad() {
  double load;
  getloadavg(&load, 1);
  load_ = std::min(load / numOfProcessors_, 1.0);
  return true;
}

}  // namespace sysapps
}  // namespace xwalk
