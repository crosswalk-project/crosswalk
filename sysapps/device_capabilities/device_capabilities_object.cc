// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/device_capabilities_object.h"

#include <string>

#include "xwalk/sysapps/common/sysapps_manager.h"
#include "xwalk/sysapps/device_capabilities/av_codecs_provider.h"
#include "xwalk/sysapps/device_capabilities/cpu_info_provider.h"
#include "xwalk/sysapps/device_capabilities/display_info_provider.h"
#include "xwalk/sysapps/device_capabilities/memory_info_provider.h"
#include "xwalk/sysapps/device_capabilities/storage_info_provider.h"

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
  handler_.Register("getDisplayInfo",
                    base::Bind(&DeviceCapabilitiesObject::OnGetDisplayInfo,
                               base::Unretained(this)));
  handler_.Register("getMemoryInfo",
                    base::Bind(&DeviceCapabilitiesObject::OnGetMemoryInfo,
                               base::Unretained(this)));
  handler_.Register("getStorageInfo",
                    base::Bind(&DeviceCapabilitiesObject::OnGetStorageInfo,
                               base::Unretained(this)));
}

DeviceCapabilitiesObject::~DeviceCapabilitiesObject() {
  if (SysAppsManager::GetStorageInfoProvider()->HasObserver(this))
    SysAppsManager::GetStorageInfoProvider()->RemoveObserver(this);

  if (SysAppsManager::GetDisplayInfoProvider()->HasObserver(this))
    SysAppsManager::GetDisplayInfoProvider()->RemoveObserver(this);
}

void DeviceCapabilitiesObject::StartEvent(const std::string& type) {
  if (type == "storageattach" || type == "storagedetach") {
    if (!SysAppsManager::GetStorageInfoProvider()->HasObserver(this))
      SysAppsManager::GetStorageInfoProvider()->AddObserver(this);
  } else if (type == "displayconnect" || type == "displaydisconnect") {
    if (!SysAppsManager::GetDisplayInfoProvider()->HasObserver(this))
      SysAppsManager::GetDisplayInfoProvider()->AddObserver(this);
  }
}

void DeviceCapabilitiesObject::StopEvent(const std::string& type) {
  if (type == "storageattach" || type == "storagedetach") {
    if (!IsEventActive("storageattach") && !IsEventActive("storagedetach"))
      SysAppsManager::GetStorageInfoProvider()->RemoveObserver(this);
  } else if (type == "displayconnect" || type == "displaydisconnect") {
    if (!IsEventActive("displayconnect") && !IsEventActive("displaydisconnect"))
      SysAppsManager::GetDisplayInfoProvider()->RemoveObserver(this);
  }
}

void DeviceCapabilitiesObject::OnDisplayConnected(const DisplayUnit& display) {
  scoped_ptr<base::ListValue> eventData(new base::ListValue);
  eventData->Append(display.ToValue().release());

  DispatchEvent("displayconnect", eventData.Pass());
}

void DeviceCapabilitiesObject::OnDisplayDisconnected(
    const DisplayUnit& display) {
  scoped_ptr<base::ListValue> eventData(new base::ListValue);
  eventData->Append(display.ToValue().release());

  DispatchEvent("displaydisconnect", eventData.Pass());
}

void DeviceCapabilitiesObject::OnStorageAttached(const StorageUnit& storage) {
  scoped_ptr<base::ListValue> eventData(new base::ListValue);
  eventData->Append(storage.ToValue().release());

  DispatchEvent("storageattach", eventData.Pass());
}

void DeviceCapabilitiesObject::OnStorageDetached(const StorageUnit& storage) {
  scoped_ptr<base::ListValue> eventData(new base::ListValue);
  eventData->Append(storage.ToValue().release());

  DispatchEvent("storagedetach", eventData.Pass());
}

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

void DeviceCapabilitiesObject::OnGetDisplayInfo(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  DisplayInfoProvider* provider(SysAppsManager::GetDisplayInfoProvider());
  scoped_ptr<SystemDisplay> display_info(provider->display_info());
  info->PostResult(GetDisplayInfo::Results::Create(*display_info,
                                                   std::string()));
}

void DeviceCapabilitiesObject::OnGetMemoryInfo(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<SystemMemory> memory_info(
      SysAppsManager::GetMemoryInfoProvider()->memory_info());
  info->PostResult(GetMemoryInfo::Results::Create(*memory_info, std::string()));
}

void DeviceCapabilitiesObject::OnGetStorageInfo(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  StorageInfoProvider* provider(SysAppsManager::GetStorageInfoProvider());

  // Queue the message if the backend is not initialized yet.
  if (!provider->IsInitialized()) {
    provider->AddOnInitCallback(
        base::Bind(&DeviceCapabilitiesObject::OnGetStorageInfo,
                   base::Unretained(this),
                   base::Passed(&info)));
    return;
  }

  scoped_ptr<SystemStorage> storage_info(provider->storage_info());
  info->PostResult(GetStorageInfo::Results::Create(
      *storage_info, std::string()));
}

}  // namespace sysapps
}  // namespace xwalk
