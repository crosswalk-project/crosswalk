// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/cpu_info_provider.h"

#include <pdh.h>

#include "base/logging.h"

namespace xwalk {
namespace sysapps {

namespace {
  PDH_HQUERY cpu_query;
  PDH_HCOUNTER cpu_total;
}

void CPUInfoProvider::init() {
  PdhOpenQuery(nullptr, NULL, &cpu_query);
  PdhAddCounter(cpu_query,
                L"\\Processor(_Total)\\% Processor Time", NULL, &cpu_total);
}

double CPUInfoProvider::GetCPULoad() const {
  PDH_FMT_COUNTERVALUE counter_value;
  PdhCollectQueryData(cpu_query);
  PdhGetFormattedCounterValue(cpu_total, PDH_FMT_DOUBLE,
                              nullptr, &counter_value);
  return counter_value.doubleValue / 100;
}

}  // namespace sysapps
}  // namespace xwalk
