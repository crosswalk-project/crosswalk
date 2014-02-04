// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/cpu_info_provider.h"

#include <string>

#include "base/file_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/sys_info.h"

namespace {

const char kProcLoadavg[] = "/proc/loadavg";

}  // namespace

namespace xwalk {
namespace sysapps {

double CPUInfoProvider::GetCPULoad() const {
  // Bionic doesn't have a getloadavg() implementation.
  const base::FilePath proc_loadavg(kProcLoadavg);
  std::string buffer;

  base::ReadFileToString(proc_loadavg, &buffer);

  std::vector<std::string> stats;
  base::SplitString(buffer, ' ', &stats);

  double load;
  base::StringToDouble(stats[0], &load);

  return std::min(load / number_of_processors_, 1.0);
}

}  // namespace sysapps
}  // namespace xwalk
