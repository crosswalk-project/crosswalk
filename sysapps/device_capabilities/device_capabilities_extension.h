// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_EXTENSION_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_EXTENSION_H_

#include <string>

#include "base/values.h"
#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/sysapps/common/binding_object_store.h"

namespace xwalk {
namespace sysapps {
namespace experimental {

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;
using extensions::XWalkExtensionInstance;

class DeviceCapabilitiesExtension : public XWalkExtension {
 public:
  DeviceCapabilitiesExtension();
  ~DeviceCapabilitiesExtension() override;

  // XWalkExtension implementation.
  XWalkExtensionInstance* CreateInstance() override;
};

class DeviceCapabilitiesInstance : public XWalkExtensionInstance {
 public:
  DeviceCapabilitiesInstance();

  // XWalkExtensionInstance implementation.
  void HandleMessage(std::unique_ptr<base::Value> msg) override;

 private:
  void OnDeviceCapabilitiesConstructor(
      std::unique_ptr<XWalkExtensionFunctionInfo> info);

  XWalkExtensionFunctionHandler handler_;
  BindingObjectStore store_;
};

}  // namespace experimental
}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_EXTENSION_H_
