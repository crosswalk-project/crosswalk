// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/memory_info_provider.h"

#include "base/sys_info.h"

namespace xwalk {
namespace sysapps {

MemoryInfoProvider::MemoryInfoProvider()
  : amount_of_physical_memory_(base::SysInfo::AmountOfPhysicalMemory()) {}

MemoryInfoProvider::~MemoryInfoProvider() {}

scoped_ptr<SystemMemory> MemoryInfoProvider::memory_info() const {
  scoped_ptr<SystemMemory> info(new SystemMemory);

  info->capacity = amount_of_physical_memory_;
  info->avail_capacity = base::SysInfo::AmountOfAvailablePhysicalMemory();

  return info.Pass();
}

}  // namespace sysapps
}  // namespace xwalk
