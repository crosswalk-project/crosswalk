// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_OBJECT_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_OBJECT_H_

#include <string>
#include "xwalk/sysapps/common/event_target.h"
#include "xwalk/sysapps/device_capabilities/display_info_provider.h"
#include "xwalk/sysapps/device_capabilities/storage_info_provider.h"

namespace xwalk {
namespace sysapps {

class DeviceCapabilitiesObject : public EventTarget,
                                 public DisplayInfoProvider::Observer,
                                 public StorageInfoProvider::Observer {
 public:
  DeviceCapabilitiesObject();
  virtual ~DeviceCapabilitiesObject();

  // EventTarget implementation.
  void StartEvent(const std::string& type) override;
  void StopEvent(const std::string& type) override;

  // DisplayInfoProvider::Observer implementation.
  void OnDisplayConnected(const DisplayUnit& display) override;
  void OnDisplayDisconnected(const DisplayUnit& display) override;

  // StorageInfoProvider::Observer implementation.
  void OnStorageAttached(const StorageUnit& storage) override;
  void OnStorageDetached(const StorageUnit& storage) override;

 private:
  void OnGetAVCodecs(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnGetCPUInfo(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnGetDisplayInfo(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnGetMemoryInfo(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnGetStorageInfo(scoped_ptr<XWalkExtensionFunctionInfo> info);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_OBJECT_H_
