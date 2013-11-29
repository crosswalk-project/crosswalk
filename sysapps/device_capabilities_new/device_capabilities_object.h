// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_DEVICE_CAPABILITIES_OBJECT_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_DEVICE_CAPABILITIES_OBJECT_H_

#include "xwalk/sysapps/common/binding_object.h"

namespace xwalk {
namespace sysapps {

class DeviceCapabilitiesObject : public BindingObject {
 public:
  DeviceCapabilitiesObject();
  virtual ~DeviceCapabilitiesObject();

 private:
  void OnGetCPUInfo(scoped_ptr<XWalkExtensionFunctionInfo> info);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_DEVICE_CAPABILITIES_OBJECT_H_
