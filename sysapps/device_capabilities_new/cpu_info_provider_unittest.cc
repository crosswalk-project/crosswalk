// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities_new/cpu_info_provider.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/sysapps/device_capabilities_new/device_capabilities.h"

using xwalk::jsapi::device_capabilities::SystemCPU;
using xwalk::sysapps::CPUInfoProvider;

TEST(XWalkSysAppsDeviceCapabilitiesTest, CPUInfoProvider) {
  scoped_ptr<CPUInfoProvider> provider(new CPUInfoProvider());
  scoped_ptr<SystemCPU> info_cached(provider->cpu_info());

  for (unsigned i = 0; i < 1000; ++i) {
    scoped_ptr<SystemCPU> info(provider->cpu_info());

    // These data should be constant across multiple calls.
    EXPECT_EQ(info->num_of_processors, info_cached->num_of_processors);
    EXPECT_EQ(info->arch_name, info_cached->arch_name);

    EXPECT_GE(info->num_of_processors, 1);
    EXPECT_GE(info->load, 0);
    EXPECT_LE(info->load, 1);
  }
}
