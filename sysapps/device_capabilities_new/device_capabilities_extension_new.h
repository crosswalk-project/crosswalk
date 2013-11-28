// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_DEVICE_CAPABILITIES_EXTENSION_NEW_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_DEVICE_CAPABILITIES_EXTENSION_NEW_H_

#include <string>
#include "base/values.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"

namespace xwalk {
namespace sysapps {
namespace experimental {

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionInstance;

class DeviceCapabilitiesExtension : public XWalkExtension {
 public:
  explicit DeviceCapabilitiesExtension();
  virtual ~DeviceCapabilitiesExtension();

  // XWalkExtension implementation.
  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;
};

class DeviceCapabilitiesInstance : public XWalkExtensionInstance {
 public:
  DeviceCapabilitiesInstance();

  // XWalkExtensionInstance implementation.
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

 private:
  XWalkExtensionFunctionHandler handler_;
};

}  // namespace experimental
}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_DEVICE_CAPABILITIES_EXTENSION_NEW_H_
