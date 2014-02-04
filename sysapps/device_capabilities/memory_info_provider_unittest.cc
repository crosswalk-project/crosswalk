// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/memory_info_provider.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities.h"

using xwalk::jsapi::device_capabilities::SystemMemory;
using xwalk::sysapps::MemoryInfoProvider;

TEST(XWalkSysAppsDeviceCapabilitiesTest, MemoryInfoProvider) {
  scoped_ptr<MemoryInfoProvider> provider(new MemoryInfoProvider());
  scoped_ptr<SystemMemory> info(provider->memory_info());

  EXPECT_GE(info->avail_capacity, 0);
  EXPECT_GE(info->capacity, 0);
  EXPECT_GE(info->capacity, info->avail_capacity);
}
