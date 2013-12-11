// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_STORAGE_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_STORAGE_H_

#include <libudev.h>

#include <map>
#include <string>

#include "base/timer/timer.h"
#include "third_party/jsoncpp/source/include/json/json.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_utils.h"

namespace xwalk {
namespace sysapps {

struct DeviceStorageUnit {
  int64 id;
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

  ~DeviceCapabilitiesStorage();

  Json::Value* Get();
  void AddEventListener(const std::string& event_name,
                        DeviceCapabilitiesInstance* instance);
  void RemoveEventListener(DeviceCapabilitiesInstance* instance);

 private:
  explicit DeviceCapabilitiesStorage();
  bool IsRealStorageDevice(udev_device *dev);
  bool InitStorageMonitor();
  DeviceStorageUnit MakeStorageUnit(udev_device *dev);
  void OnStorageHotplug();
  void PostbackStorageUnit(const DeviceStorageUnit& unit,
                           const std::string& reply,
                           const std::string& event_name,
                           const int action);
  void QueryStorageUnits();
  void SetJsonValue(Json::Value* obj, const DeviceStorageUnit& unit);

  udev* udev_;
  udev_monitor* udev_monitor_;
  int udev_monitor_fd_;
  udev_enumerate* enumerate_;
  base::RepeatingTimer<DeviceCapabilitiesStorage> timer_;

  typedef std::map<int64, DeviceStorageUnit> StoragesMap;
  StoragesMap storages_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_STORAGE_H_
