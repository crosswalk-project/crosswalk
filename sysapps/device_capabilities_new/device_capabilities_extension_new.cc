// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities_new/device_capabilities_extension_new.h"

#include "grit/xwalk_sysapps_resources.h"
#include "ui/base/resource/resource_bundle.h"

namespace xwalk {
namespace sysapps {
namespace experimental {

DeviceCapabilitiesExtension::DeviceCapabilitiesExtension() {
  set_name("xwalk.experimental.system");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_API).as_string());
}

DeviceCapabilitiesExtension::~DeviceCapabilitiesExtension() {}

XWalkExtensionInstance* DeviceCapabilitiesExtension::CreateInstance() {
  return new DeviceCapabilitiesInstance();
}

DeviceCapabilitiesInstance::DeviceCapabilitiesInstance()
  : handler_(this) {}

void DeviceCapabilitiesInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  handler_.HandleMessage(msg.Pass());
}

}  // namespace experimental
}  // namespace sysapps
}  // namespace xwalk
