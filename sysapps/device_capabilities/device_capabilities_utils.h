// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_UTILS_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_UTILS_H_

#include <list>
#include <string>

#include "third_party/jsoncpp/source/include/json/json.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_instance.h"

namespace xwalk {
namespace sysapps {

typedef std::list<DeviceCapabilitiesInstance*> DeviceCapabilitiesEventsList;

class DeviceCapabilitiesObject {
 public:
  virtual Json::Value* Get() = 0;
  virtual void AddEventListener(const std::string& event_name,
                                DeviceCapabilitiesInstance* instance) = 0;
  virtual void RemoveEventListener(DeviceCapabilitiesInstance* instance) = 0;

  void PostMessageToAllListeners(const DeviceCapabilitiesEventsList& listeners,
                                 const char* result) {
    DeviceCapabilitiesEventsList::const_iterator it = listeners.begin();
    for (; it != listeners.end(); it++) {
      (*it)->PostMessage(result);
    }
  }

 protected:
  DeviceCapabilitiesEventsList attach_listeners_;
  DeviceCapabilitiesEventsList detach_listeners_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_UTILS_H_
