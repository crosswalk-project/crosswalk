// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/device_capabilities_storage.h"

#include <linux/netlink.h>

#include "base/strings/string_number_conversions.h"

namespace xwalk {
namespace sysapps {

DeviceCapabilitiesStorage::DeviceCapabilitiesStorage()
     : udev_(udev_new()),
       udev_monitor_(NULL),
       udev_monitor_fd_(-1),
       enumerate_(NULL) {
  InitStorageMonitor();
  QueryStorageUnits();
}

DeviceCapabilitiesStorage::~DeviceCapabilitiesStorage() {
  if (udev_)
    udev_unref(udev_);
  if (udev_monitor_)
    udev_monitor_unref(udev_monitor_);
  if (enumerate_)
    udev_enumerate_unref(enumerate_);
}

void DeviceCapabilitiesStorage::SetJsonValue(Json::Value* obj,
    const DeviceStorageUnit& unit) {
  (*obj)["id"] = Json::Value(base::Uint64ToString(unit.id));
  (*obj)["name"] = Json::Value(unit.name);
  (*obj)["type"] = Json::Value(unit.type);
  (*obj)["capacity"] = Json::Value(unit.capacity);
}

Json::Value* DeviceCapabilitiesStorage::Get() {
  Json::Value* obj = new Json::Value();
  Json::Value storages;

  for (StoragesMap::iterator it = storages_.begin();
       it != storages_.end(); it++) {
    Json::Value unit;
    SetJsonValue(&unit, it->second);
    storages.append(unit);
  }

  (*obj)["storages"] = storages;
  return obj;
}

bool DeviceCapabilitiesStorage::InitStorageMonitor() {
  if (!udev_)
    return false;

  // Init udev_monitor_.
  udev_monitor_ = udev_monitor_new_from_netlink(udev_, "udev");
  if (!udev_monitor_)
    return false;
  udev_monitor_filter_add_match_subsystem_devtype(udev_monitor_,
                                                  "block",
                                                  NULL);
  udev_monitor_enable_receiving(udev_monitor_);
  udev_monitor_fd_ = udev_monitor_get_fd(udev_monitor_);

  // Init enumerate_.
  enumerate_ = udev_enumerate_new(udev_);
  if (!enumerate_)
    return false;
  udev_enumerate_add_match_subsystem(enumerate_, "block");

  return true;
}

void DeviceCapabilitiesStorage::AddEventListener(const std::string& event_name,
    DeviceCapabilitiesInstance* instance) {
  if (event_name == "storageattach")
    attach_listeners_.push_back(instance);
  else
    detach_listeners_.push_back(instance);

  if ((attach_listeners_.size() +
       detach_listeners_.size()) == 1) {
    timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(200),
        this, &DeviceCapabilitiesStorage::OnStorageHotplug);
  }
}

void DeviceCapabilitiesStorage::RemoveEventListener(
    DeviceCapabilitiesInstance* instance) {
  attach_listeners_.remove(instance);
  detach_listeners_.remove(instance);
  if (attach_listeners_.empty() && detach_listeners_.empty()) {
    timer_.Stop();
  }
}

void DeviceCapabilitiesStorage::OnStorageHotplug() {
  if (udev_monitor_ == NULL || udev_monitor_fd_ < 0)
    return;

  fd_set fds;
  timeval tv;

  FD_ZERO(&fds);
  FD_SET(udev_monitor_fd_, &fds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  // select() fouction status:
  // 1. "block" if tv is NULL
  // 2. "non-block" if tv=={0,0}
  // 3. "block" in tv timespan and "non-block" after tv timeout if tv!={0,0}
  int ret = select(udev_monitor_fd_ + 1, &fds, NULL, NULL, &tv);
  if (!(ret >0 && FD_ISSET(udev_monitor_fd_, &fds)))
    return;

  udev_device* dev = udev_monitor_receive_device(udev_monitor_);
  if (!dev)
    return;

  std::string reply;
  std::string event_name;
  int action = -1;
  int64 dev_id = udev_device_get_devnum(dev);
  std::string dev_action = udev_device_get_action(dev);
  if (dev_action == "add") {
    if (IsRealStorageDevice(dev)) {
      reply = "attachStorage";
      event_name = "storageattach";
      action = 0;
      DeviceStorageUnit unit = MakeStorageUnit(dev);
      storages_[unit.id] = unit;
      PostbackStorageUnit(unit, reply, event_name, action);
    }
  }
  if (dev_action == "remove") {
    for (StoragesMap::iterator it = storages_.begin();
         it != storages_.end(); it++) {
      DeviceStorageUnit unit = it->second;
      if (unit.id == dev_id) {
        reply = "detachStorage";
        event_name = "storagedetach";
        action = 1;
        storages_.erase(unit.id);
        PostbackStorageUnit(unit, reply, event_name, action);
        break;
      }
    }
  }
}

bool DeviceCapabilitiesStorage::IsRealStorageDevice(udev_device *dev) {
  const char* dev_type =
      udev_device_get_sysattr_value(dev, "removable");
  const char* dev_capability =
      udev_device_get_sysattr_value(dev, "capability");
  const char* dev_size =
      udev_device_get_sysattr_value(dev, "size");

  if (dev_capability != NULL && dev_type != NULL && dev_size != NULL) {
    int capability = -1;
    base::StringToInt(dev_capability, &capability);

    // Capability meaning:
    // "50": 1. disk device for Linux, such as "sda" "sdb";
    //       2. sd-card and disk device for tizen, such as "mmcblk0" "mmcblk1"
    // "51": U-flash device for Linux
    if (capability == 50 || capability == 51)
      return true;
  }

  return false;
}

void
DeviceCapabilitiesStorage::PostbackStorageUnit(const DeviceStorageUnit& unit,
                                               const std::string& reply,
                                               const std::string& event_name,
                                               const int action) {
  Json::Value output;
  Json::Value data;
  Json::StyledWriter writer;
  std::string result;

  output["reply"] = Json::Value(reply);
  output["eventName"] = Json::Value(event_name);
  SetJsonValue(&data, unit);
  output["data"] = data;
  result = writer.write(output);
  if (action == 0) {
    PostMessageToAllListeners(attach_listeners_, result.c_str());
  } else if (action == 1) {
    PostMessageToAllListeners(detach_listeners_, result.c_str());
  }
}

DeviceStorageUnit
DeviceCapabilitiesStorage::MakeStorageUnit(udev_device* dev) {
  int64 dev_id = udev_device_get_devnum(dev);
  std::string dev_name = udev_device_get_devnode(dev);
  std::string dev_type =
      udev_device_get_sysattr_value(dev, "removable");
  std::string dev_size =
      udev_device_get_sysattr_value(dev, "size");

  DeviceStorageUnit unit;
  unit.id = dev_id;
  unit.name = dev_name;
  int type = -1;
  base::StringToInt(dev_type, &type);
  if (type == 0) {
    unit.type = "fixed";
  } else if (type == 1) {
    unit.type = "removable";
  } else {
    unit.type = "unknown";
  }
  double capacity = 0.0;
  base::StringToDouble(dev_size, &capacity);
  unit.capacity = capacity;
  return unit;
}

void DeviceCapabilitiesStorage::QueryStorageUnits() {
  udev_list_entry* devices;
  udev_list_entry* dev_list_entry;
  udev_device* dev;

  udev_enumerate_scan_devices(enumerate_);
  devices = udev_enumerate_get_list_entry(enumerate_);
  udev_list_entry_foreach(dev_list_entry, devices) {
    const char* path = udev_list_entry_get_name(dev_list_entry);
    dev = udev_device_new_from_syspath(udev_, path);

    if (!IsRealStorageDevice(dev))
      continue;

    DeviceStorageUnit unit = MakeStorageUnit(dev);
    storages_[unit.id] = unit;
  }
}

}  // namespace sysapps
}  // namespace xwalk
