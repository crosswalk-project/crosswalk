// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_MEMORY_INFO_PROVIDER_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_MEMORY_INFO_PROVIDER_H_

#include "xwalk/sysapps/device_capabilities/device_capabilities.h"

#include <memory>

namespace xwalk {
namespace sysapps {

using jsapi::device_capabilities::SystemMemory;

class MemoryInfoProvider {
 public:
  MemoryInfoProvider();
  ~MemoryInfoProvider();

  std::unique_ptr<SystemMemory> memory_info() const;

 private:
  double amount_of_physical_memory_;

  DISALLOW_COPY_AND_ASSIGN(MemoryInfoProvider);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_MEMORY_INFO_PROVIDER_H_
