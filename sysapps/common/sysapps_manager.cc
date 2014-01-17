// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/sysapps_manager.h"

#include "base/basictypes.h"
#include "xwalk/sysapps/device_capabilities_new/cpu_info_provider.h"
#include "xwalk/sysapps/device_capabilities_new/device_capabilities_extension_new.h"
#include "xwalk/sysapps/device_capabilities_new/display_info_provider.h"
#include "xwalk/sysapps/device_capabilities_new/memory_info_provider.h"
#include "xwalk/sysapps/device_capabilities_new/storage_info_provider_mock.h"
#include "xwalk/sysapps/raw_socket/raw_socket_extension.h"

namespace xwalk {
namespace sysapps {

SysAppsManager::SysAppsManager()
    : device_capabilities_enabled_(true),
      raw_sockets_enabled_(true) {}

SysAppsManager::~SysAppsManager() {}

void SysAppsManager::DisableDeviceCapabilities() {
  device_capabilities_enabled_ = false;
}

void SysAppsManager::DisableRawSockets() {
  raw_sockets_enabled_ = false;
}

void SysAppsManager::CreateExtensionsForUIThread(
    XWalkExtensionVector* extensions) {
  // FIXME(tmpsantos): Device Capabilities needs to be in the UI Thread because
  // it uses Chromium's StorageMonitor, which requires that. We can move it back
  // to the ExtensionThread if we make StorageMonitor a truly self-contained
  // module on Chromium upstream.
  if (device_capabilities_enabled_)
    extensions->push_back(new experimental::DeviceCapabilitiesExtension());
}

void SysAppsManager::CreateExtensionsForExtensionThread(
    XWalkExtensionVector* extensions) {
  if (raw_sockets_enabled_)
    extensions->push_back(new RawSocketExtension());
}

// static
CPUInfoProvider* SysAppsManager::GetCPUInfoProvider() {
  CR_DEFINE_STATIC_LOCAL(CPUInfoProvider, provider, ());

  return &provider;
}

// static
DisplayInfoProvider* SysAppsManager::GetDisplayInfoProvider() {
  CR_DEFINE_STATIC_LOCAL(DisplayInfoProvider, provider, ());

  return &provider;
}

// static
MemoryInfoProvider* SysAppsManager::GetMemoryInfoProvider() {
  CR_DEFINE_STATIC_LOCAL(MemoryInfoProvider, provider, ());

  return &provider;
}

// static
StorageInfoProvider* SysAppsManager::GetStorageInfoProvider() {
  CR_DEFINE_STATIC_LOCAL(StorageInfoProviderMock, provider, ());

  return &provider;
}

}  // namespace sysapps
}  // namespace xwalk
