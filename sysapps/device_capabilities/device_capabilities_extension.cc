// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/device_capabilities_extension.h"

#include "grit/xwalk_sysapps_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_instance.h"

namespace xwalk {
namespace sysapps {

DeviceCapabilitiesExtension::DeviceCapabilitiesExtension() {
  set_name("xwalk.experimental.system");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_SYSAPPS_DEVICE_CAPABILITIES_API).as_string());
  DeviceCapabilitiesInstance::DeviceMapInitialize();
}

DeviceCapabilitiesExtension::~DeviceCapabilitiesExtension() {
}

XWalkExtensionInstance* DeviceCapabilitiesExtension::CreateInstance() {
  return new DeviceCapabilitiesInstance;
}

}  // namespace sysapps
}  // namespace xwalk
