// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_STORAGE_INFO_PROVIDER_CHROMIUM_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_STORAGE_INFO_PROVIDER_CHROMIUM_H_

#include "components/storage_monitor/removable_storage_observer.h"
#include "xwalk/sysapps/device_capabilities_new/storage_info_provider.h"

namespace xwalk {
namespace sysapps {

class StorageInfoProviderChromium : public StorageInfoProvider,
                                    public RemovableStorageObserver {
 public:
  StorageInfoProviderChromium();
  virtual ~StorageInfoProviderChromium();

  virtual scoped_ptr<SystemStorage> storage_info() const;

  // RemovableStorageObserver implementation.
  virtual void OnRemovableStorageAttached(const StorageInfo& info) OVERRIDE;
  virtual void OnRemovableStorageDetached(const StorageInfo& info) OVERRIDE;

 private:
  // StorageInfoProvider implementation.
  virtual void StartStorageMonitoring() OVERRIDE;
  virtual void StopStorageMonitoring() OVERRIDE;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_STORAGE_INFO_PROVIDER_CHROMIUM_H_
