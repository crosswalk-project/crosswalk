// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_INSTANCE_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_INSTANCE_H_

#include <string>

#include "base/values.h"
#include "third_party/jsoncpp/source/include/json/json.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {
namespace sysapps {

using extensions::XWalkExtensionInstance;

class DeviceCapabilitiesInstance : public XWalkExtensionInstance {
 public:
  explicit DeviceCapabilitiesInstance();
  virtual ~DeviceCapabilitiesInstance();
  static void DeviceMapInitialize();

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;
  void PostMessage(const char* msg);

 private:
  void HandleGetDeviceInfo(std::string deviceName, const Json::Value& msg);
  void HandleAddEventListener(const Json::Value& msg);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_INSTANCE_H_
