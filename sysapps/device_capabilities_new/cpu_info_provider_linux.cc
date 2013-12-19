// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities_new/cpu_info_provider.h"

#include <stdlib.h>
#include "base/sys_info.h"

namespace xwalk {
namespace sysapps {

double CPUInfoProvider::GetCPULoad() const {
  double load;
  getloadavg(&load, 1);

  return std::min(load / number_of_processors_, 1.0);
}

}  // namespace sysapps
}  // namespace xwalk
