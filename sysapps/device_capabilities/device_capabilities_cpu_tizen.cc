// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/device_capabilities_cpu.h"

#include <device.h>
#include <sys/utsname.h>

#include "base/logging.h"

namespace xwalk {
namespace sysapps {

DeviceCapabilitiesCpu::DeviceCapabilitiesCpu()
    : numOfProcessors_(0),
      archName_("Unknown"),
      load_(0.0),
      old_total_(0),
      old_used_(0) {
  if (device_cpu_get_count(&numOfProcessors_) != DEVICE_ERROR_NONE) {
    LOG(ERROR) << "get CPU count failed";
  }

  struct utsname utsbuf;
  if (uname(&utsbuf) == -1) {
    LOG(ERROR) << "uname failed";
  } else {
    archName_ = std::string(utsbuf.machine);
  }
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
  FILE* fp = fopen("/proc/stat", "r");
  if (!fp)
    return false;

  unsigned long long user; //NOLINT
  unsigned long long nice; //NOLINT
  unsigned long long system; //NOLINT
  unsigned long long idle; //NOLINT
  unsigned long long iowait; //NOLINT
  unsigned long long irq; //NOLINT
  unsigned long long softirq; //NOLINT

  unsigned long long total; //NOLINT
  unsigned long long used; //NOLINT

  if (fscanf(fp, "%*s %llu %llu %llu %llu %llu %llu %llu",
             &user, &nice, &system, &idle, &iowait, &irq, &softirq) != 7) {
      fclose(fp);
      return false;
  }
  fclose(fp);

  // The algorithm here can be found at:
  // http://stackoverflow.com/questions/3017162
  // /how-to-get-total-cpu-usage-in-linux-c
  used = user + nice + system;
  total = used + idle + iowait + irq + softirq;
  if (total == old_total_) {
    load_ = 0.0;
  } else {
    load_ = static_cast<double>(used - old_used_) / (total - old_total_);
    old_total_ = total;
  }
  old_used_ = used;
  return true;
}

}  // namespace sysapps
}  // namespace xwalk
