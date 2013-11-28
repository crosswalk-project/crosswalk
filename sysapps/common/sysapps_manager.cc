// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/sysapps_manager.h"

#include "xwalk/runtime/common/xwalk_runtime_features.h"
#include "xwalk/sysapps/device_capabilities_new/device_capabilities_extension_new.h"
#include "xwalk/sysapps/raw_socket/raw_socket_extension.h"

namespace xwalk {
namespace sysapps {

SysAppsManager::SysAppsManager() {}

SysAppsManager::~SysAppsManager() {}

void SysAppsManager::CreateExtensionsForUIThread(
    XWalkExtensionVector* extensions) {
  if (!XWalkRuntimeFeatures::isSysAppsEnabled())
    return;

  // FIXME(tmpsantos): Device Capabilities needs to be in the UI Thread because
  // it uses Chromium's StorageMonitor, which requires that. We can move it back
  // to the ExtensionThread if we make StorageMonitor a truly self-contained
  // module on Chromium upstream.
  if (XWalkRuntimeFeatures::isDeviceCapabilitiesAPIEnabled())
    extensions->push_back(new experimental::DeviceCapabilitiesExtension());
}

void SysAppsManager::CreateExtensionsForExtensionThread(
    XWalkExtensionVector* extensions) {
  if (!XWalkRuntimeFeatures::isSysAppsEnabled())
    return;

  if (XWalkRuntimeFeatures::isRawSocketsAPIEnabled())
    extensions->push_back(new RawSocketExtension());
}

}  // namespace sysapps
}  // namespace xwalk
