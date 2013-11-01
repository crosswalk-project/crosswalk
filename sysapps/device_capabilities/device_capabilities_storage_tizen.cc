// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/device_capabilities_storage.h"

#include <sys/statfs.h>

#include <sstream>

#include "base/logging.h"

namespace xwalk {
namespace sysapps {

DeviceCapabilitiesStorage::DeviceCapabilitiesStorage() {
  DeviceStorageUnit internalUnit, mmcUnit;
  if (QueryStorage("Internal", internalUnit)) {
    storages_[internalUnit.id] = internalUnit;
  }
  if (QueryStorage("MMC", mmcUnit)) {
    storages_[mmcUnit.id] = mmcUnit;
  }
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

void DeviceCapabilitiesStorage::AddEventListener(const std::string& event_name,
    DeviceCapabilitiesInstance* instance) {
  if (event_name == "onattach")
    attach_listeners_.push_back(instance);
  else
    detach_listeners_.push_back(instance);

  if ((attach_listeners_.size() +
       detach_listeners_.size()) == 1) {
    vconf_notify_key_changed(VCONFKEY_SYSMAN_MMC_STATUS,
        (vconf_callback_fn)OnStorageStatusChanged, this);
  }
}

void DeviceCapabilitiesStorage::RemoveEventListener(
    DeviceCapabilitiesInstance* instance) {
  attach_listeners_.remove(instance);
  detach_listeners_.remove(instance);
  if (attach_listeners_.empty() && detach_listeners_.empty()) {
    vconf_ignore_key_changed(VCONFKEY_SYSMAN_MMC_STATUS,
        (vconf_callback_fn)OnStorageStatusChanged);
  }
}

bool DeviceCapabilitiesStorage::QueryStorage(const std::string& type,
                                             DeviceStorageUnit& unit) {
  struct statfs fs;
  if (type == "Internal") {
    if (statfs("/opt/usr/media", &fs) < 0) {
      LOG(ERROR) << "Internal Storage path Error";
      return false;
    }
    unit.type = std::string("fixed");
  } else {
    int sdcard_state;
    if ((vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &sdcard_state) != 0) ||
        (sdcard_state != VCONFKEY_SYSMAN_MMC_MOUNTED)) {
      LOG(ERROR) << "Failed to read SD card status";
      return false;
    }

    if (statfs("/opt/storage/sdcard", &fs) < 0) {
      LOG(ERROR) << "MMC mount path error";
      return false;
    }
    unit.type = std::string("removable");
  }

  memcpy(&unit.id, &fs.f_fsid, sizeof(unit.id));
  // FIXME(qjia7): find which field reflects 'name'
  unit.name = "";
  unit.capacity = static_cast<double>(fs.f_bsize) *
                  static_cast<double>(fs.f_blocks);
  return true;
}

void DeviceCapabilitiesStorage::SetJsonValue(Json::Value* obj,
                                             const DeviceStorageUnit& unit) {
  std::stringstream id_string;
  id_string << unit.id;
  (*obj)["id"] = Json::Value(id_string.str());
  (*obj)["name"] = Json::Value(unit.name);
  (*obj)["type"] = Json::Value(unit.type);
  (*obj)["capacity"] = Json::Value(unit.capacity);
}

void DeviceCapabilitiesStorage::UpdateStorageUnits(std::string command) {
  Json::Value output;
  Json::Value data;
  Json::StyledWriter writer;
  std::string result;

  output["reply"] = Json::Value(command);
  DeviceStorageUnit mmcUnit;
  if (command == "attachStorage" && QueryStorage("MMC", mmcUnit)) {
    output["eventName"] = Json::Value("onattach");
    SetJsonValue(&data, mmcUnit);
    storages_[mmcUnit.id] = mmcUnit;
    output["data"] = data;
    result = writer.write(output);
    PostMessageToAllListeners(attach_listeners_, result.c_str());
    return;
  }

  for (StoragesMap::iterator it = storages_.begin();
       it != storages_.end(); it++) {
    DeviceStorageUnit unit = it->second;
    if (unit.type == "removable") {
      output["eventName"] = Json::Value("ondetach");
      SetJsonValue(&data, unit);
      storages_.erase(it);
      output["data"] = data;
      result = writer.write(output);
      PostMessageToAllListeners(detach_listeners_, result.c_str());
      break;
    }
  }
}

void DeviceCapabilitiesStorage::OnStorageStatusChanged(keynode_t* node,
                                                       void* user_data) {
  int status = vconf_keynode_get_int(node);
  DeviceCapabilitiesStorage* instance =
      static_cast<DeviceCapabilitiesStorage*>(user_data);
  if (status == 1) {
    instance->UpdateStorageUnits("attachStorage");
  } else {
    instance->UpdateStorageUnits("detachStorage");
  }
}

}  // namespace sysapps
}  // namespace xwalk
