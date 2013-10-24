// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/device_capabilities_instance.h"

#include <map>
#include <utility>

#include "base/logging.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_cpu.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_display.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_memory.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_storage.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_utils.h"

namespace xwalk {
namespace sysapps {

typedef std::map<const std::string, DeviceCapabilitiesObject&> DeviceMap;
typedef std::pair<const std::string, DeviceCapabilitiesObject&> DeviceMapPair;

static DeviceMap device_map_;

DeviceCapabilitiesInstance::DeviceCapabilitiesInstance() {
}

DeviceCapabilitiesInstance::~DeviceCapabilitiesInstance() {
  for (DeviceMap::iterator it = device_map_.begin();
       it != device_map_.end(); it++) {
    (it->second).RemoveEventListener(this);
  }
}

void DeviceCapabilitiesInstance::DeviceMapInitialize() {
  device_map_.insert(DeviceMapPair(
      "CPU", DeviceCapabilitiesCpu::GetDeviceInstance()));
  device_map_.insert(DeviceMapPair(
      "Display", DeviceCapabilitiesDisplay::GetDeviceInstance()));
  device_map_.insert(DeviceMapPair(
      "Memory", DeviceCapabilitiesMemory::GetDeviceInstance()));
  device_map_.insert(DeviceMapPair(
      "Storage", DeviceCapabilitiesStorage::GetDeviceInstance()));
}

void DeviceCapabilitiesInstance::PostMessage(const char* msg) {
  PostMessageToJS(scoped_ptr<base::Value>(new base::StringValue(msg)));
}

void DeviceCapabilitiesInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  std::string message;
  msg->GetAsString(&message);
  Json::Value input;
  Json::Reader reader;
  if (!reader.parse(message, input)) {
    LOG(ERROR) << "Failed to parse message: " << message;
    return;
  }
  std::string cmd = input["cmd"].asString();

  if (cmd == "getCPUInfo") {
    HandleGetDeviceInfo("CPU", input);
  } else if (cmd == "getMemoryInfo") {
    HandleGetDeviceInfo("Memory", input);
  } else if (cmd == "getStorageInfo") {
    HandleGetDeviceInfo("Storage", input);
  } else if (cmd == "getDisplayInfo") {
    HandleGetDeviceInfo("Display", input);
  } else if (cmd == "addEventListener") {
    HandleAddEventListener(input);
  }
}

void DeviceCapabilitiesInstance::HandleGetDeviceInfo(std::string deviceName,
                                                     const Json::Value& msg) {
  std::string reply_id = msg["_promise_id"].asString();
  Json::Value* output = new Json::Value();
  (*output)["_promise_id"] = Json::Value(reply_id);

  DeviceMap::iterator it = device_map_.find(deviceName);
  if (it == device_map_.end()) {
    LOG(ERROR) << "Invalid device name:" << deviceName;
    return;
  }

  (*output)["data"] = *((it->second).Get());
  Json::StyledWriter writer;
  std::string result = writer.write(*output);
  PostMessage(result.c_str());
}

void
DeviceCapabilitiesInstance::HandleAddEventListener(const Json::Value& msg) {
  std::string event_name = msg["eventName"].asString();
  DeviceMap::iterator it;
  if (event_name == "onattach" || event_name == "ondetach") {
    it = device_map_.find("Storage");
    if (it != device_map_.end()) {
      (it->second).AddEventListener(event_name, this);
    }
  } else if (event_name == "onconnect" || event_name == "ondisconnect") {
    it = device_map_.find("Display");
    if (it != device_map_.end()) {
      (it->second).AddEventListener(event_name, this);
    }
  }
}

}  // namespace sysapps
}  // namespace xwalk
