// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_DEVICE_CAPABILITIES_OBJECT_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_DEVICE_CAPABILITIES_OBJECT_H_

#include <string>
#include "xwalk/sysapps/common/event_target.h"
#include "xwalk/sysapps/device_capabilities_new/storage_info_provider.h"

namespace xwalk {
namespace sysapps {

class DeviceCapabilitiesObject : public EventTarget,
                                 public StorageInfoProvider::Observer {
 public:
  DeviceCapabilitiesObject();
  virtual ~DeviceCapabilitiesObject();

  // EventTarget implementation.
  virtual void StartEvent(const std::string& type) OVERRIDE;
  virtual void StopEvent(const std::string& type) OVERRIDE;

  // StorageInfoProvider::Observer implementation.
  virtual void OnStorageAttached(const StorageUnit& storage) OVERRIDE;
  virtual void OnStorageDetached(const StorageUnit& storage) OVERRIDE;

 private:
  void OnGetAVCodecs(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnGetCPUInfo(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnGetMemoryInfo(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnGetStorageInfo(scoped_ptr<XWalkExtensionFunctionInfo> info);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_DEVICE_CAPABILITIES_OBJECT_H_
