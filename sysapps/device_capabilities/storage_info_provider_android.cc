// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/storage_info_provider_android.h"

#include "base/memory/ptr_util.h"

namespace xwalk {
namespace sysapps {

StorageInfoProviderAndroid::StorageInfoProviderAndroid() {
  MarkInitialized();
}

StorageInfoProviderAndroid::~StorageInfoProviderAndroid() {}

std::unique_ptr<SystemStorage> StorageInfoProviderAndroid::storage_info() const {
  NOTIMPLEMENTED();
  return base::WrapUnique(new SystemStorage);
}

void StorageInfoProviderAndroid::StartStorageMonitoring() {
  NOTIMPLEMENTED();
}

void StorageInfoProviderAndroid::StopStorageMonitoring() {
  NOTIMPLEMENTED();
}

}  // namespace sysapps
}  // namespace xwalk
