// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_STORAGE_INFO_PROVIDER_ANDROID_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_STORAGE_INFO_PROVIDER_ANDROID_H_

#include "xwalk/sysapps/device_capabilities/storage_info_provider.h"

namespace xwalk {
namespace sysapps {

class StorageInfoProviderAndroid : public StorageInfoProvider {
 public:
  StorageInfoProviderAndroid();
  ~StorageInfoProviderAndroid() override;

  std::unique_ptr<SystemStorage> storage_info() const override;

 private:
  // StorageInfoProvider implementation.
  void StartStorageMonitoring() override;
  void StopStorageMonitoring() override;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_STORAGE_INFO_PROVIDER_ANDROID_H_
