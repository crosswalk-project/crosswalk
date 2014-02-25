// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/storage_info_provider_android.h"

namespace xwalk {
namespace sysapps {

StorageInfoProviderAndroid::StorageInfoProviderAndroid() {
  MarkInitialized();
}

StorageInfoProviderAndroid::~StorageInfoProviderAndroid() {}

scoped_ptr<SystemStorage> StorageInfoProviderAndroid::storage_info() const {
  NOTIMPLEMENTED();
  return make_scoped_ptr(new SystemStorage);
}

void StorageInfoProviderAndroid::StartStorageMonitoring() {
  NOTIMPLEMENTED();
}

void StorageInfoProviderAndroid::StopStorageMonitoring() {
  NOTIMPLEMENTED();
}

}  // namespace sysapps
}  // namespace xwalk
