// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/cpu_info_provider.h"

#include "base/sys_info.h"

namespace xwalk {
namespace sysapps {

CPUInfoProvider::CPUInfoProvider()
    : number_of_processors_(base::SysInfo::NumberOfProcessors()),
      processor_architecture_(base::SysInfo::OperatingSystemArchitecture()) {
}

CPUInfoProvider::~CPUInfoProvider() {}

scoped_ptr<SystemCPU> CPUInfoProvider::cpu_info() const {
  scoped_ptr<SystemCPU> info(new SystemCPU);

  info->num_of_processors = number_of_processors_;
  info->arch_name = processor_architecture_;
  info->load = GetCPULoad();

  return info.Pass();
}

}  // namespace sysapps
}  // namespace xwalk
