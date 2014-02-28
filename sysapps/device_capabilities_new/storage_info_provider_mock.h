// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_STORAGE_INFO_PROVIDER_MOCK_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_STORAGE_INFO_PROVIDER_MOCK_H_

#include "base/timer/timer.h"
#include "xwalk/sysapps/device_capabilities_new/storage_info_provider.h"

namespace xwalk {
namespace sysapps {

class StorageInfoProviderMock : public StorageInfoProvider {
 public:
  StorageInfoProviderMock();
  virtual ~StorageInfoProviderMock();

  virtual scoped_ptr<SystemStorage> storage_info() const OVERRIDE;

 private:
  // StorageInfoProvider implementation.
  virtual void StartStorageMonitoring() OVERRIDE;
  virtual void StopStorageMonitoring() OVERRIDE;

  void MockStorageEvent();

  base::RepeatingTimer<StorageInfoProviderMock> timer_;
  bool is_removable_storage_present;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_STORAGE_INFO_PROVIDER_MOCK_H_
