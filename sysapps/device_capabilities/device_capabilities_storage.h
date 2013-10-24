// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_STORAGE_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_STORAGE_H_

#include <vconf.h>

#include <map>
#include <string>

#include "third_party/jsoncpp/source/include/json/json.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_utils.h"

namespace xwalk {
namespace sysapps {

struct DeviceStorageUnit {
  unsigned int id;
  std::string name;
  std::string type;
  double capacity;
};

class DeviceCapabilitiesStorage : public DeviceCapabilitiesObject {
 public:
  static DeviceCapabilitiesObject& GetDeviceInstance() {
    static DeviceCapabilitiesStorage instance;
    return instance;
  }
  Json::Value* Get();
  void AddEventListener(const std::string& event_name,
                        DeviceCapabilitiesInstance* instance);
  void RemoveEventListener(DeviceCapabilitiesInstance* instance);

 private:
  explicit DeviceCapabilitiesStorage();

  void SetJsonValue(Json::Value* obj, const DeviceStorageUnit& unit);
  bool QueryStorage(const std::string& type, DeviceStorageUnit& unit);
  void UpdateStorageUnits(std::string command);
  static void OnStorageStatusChanged(keynode_t* node, void* user_data);

  typedef std::map<unsigned int, DeviceStorageUnit> StoragesMap;
  StoragesMap storages_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_STORAGE_H_
