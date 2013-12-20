// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities_new/device_capabilities_object.h"

#include <string>

#include "xwalk/sysapps/common/sysapps_manager.h"
#include "xwalk/sysapps/device_capabilities_new/av_codecs_provider.h"
#include "xwalk/sysapps/device_capabilities_new/cpu_info_provider.h"
#include "xwalk/sysapps/device_capabilities_new/memory_info_provider.h"

namespace xwalk {
namespace sysapps {

using namespace jsapi::device_capabilities; // NOLINT

DeviceCapabilitiesObject::DeviceCapabilitiesObject() {
  handler_.Register("getAVCodecs",
                    base::Bind(&DeviceCapabilitiesObject::OnGetAVCodecs,
                               base::Unretained(this)));
  handler_.Register("getCPUInfo",
                    base::Bind(&DeviceCapabilitiesObject::OnGetCPUInfo,
                               base::Unretained(this)));
  handler_.Register("getMemoryInfo",
                    base::Bind(&DeviceCapabilitiesObject::OnGetMemoryInfo,
                               base::Unretained(this)));
}

DeviceCapabilitiesObject::~DeviceCapabilitiesObject() {}

void DeviceCapabilitiesObject::OnGetAVCodecs(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<SystemAVCodecs> av_codecs(
      SysAppsManager::GetAVCodecsProvider()->GetSupportedCodecs());
  info->PostResult(GetAVCodecs::Results::Create(*av_codecs, std::string()));
}

void DeviceCapabilitiesObject::OnGetCPUInfo(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<SystemCPU> cpu_info(
      SysAppsManager::GetCPUInfoProvider()->cpu_info());
  info->PostResult(GetCPUInfo::Results::Create(*cpu_info, std::string()));
}

void DeviceCapabilitiesObject::OnGetMemoryInfo(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<SystemMemory> memory_info(
      SysAppsManager::GetMemoryInfoProvider()->memory_info());
  info->PostResult(GetMemoryInfo::Results::Create(*memory_info, std::string()));
}

}  // namespace sysapps
}  // namespace xwalk
