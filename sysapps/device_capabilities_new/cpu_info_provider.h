// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_CPU_INFO_PROVIDER_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_CPU_INFO_PROVIDER_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "xwalk/sysapps/device_capabilities_new/device_capabilities.h"

namespace xwalk {
namespace sysapps {

using jsapi::device_capabilities::SystemCPU;

class CPUInfoProvider {
 public:
  CPUInfoProvider();
  ~CPUInfoProvider();

  scoped_ptr<SystemCPU> cpu_info() const;

 private:
  // This is calculated from the average number of tasks in the
  // OS task queue divided by the number of CPUs in a 1 minute
  // window. The spec is not strict about how to calculate this,
  // so we use getloadavg(), which is avaliable on Linux, Mac and
  // Android (via /proc/loadavg).
  double GetCPULoad() const;

  int number_of_processors_;
  std::string processor_architecture_;

  DISALLOW_COPY_AND_ASSIGN(CPUInfoProvider);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_CPU_INFO_PROVIDER_H_
