// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/sysapps_manager.h"

#include "base/basictypes.h"
#include "xwalk/sysapps/device_capabilities/av_codecs_provider_android.h"
#include "xwalk/sysapps/device_capabilities/storage_info_provider_android.h"

namespace xwalk {
namespace sysapps {

// static
AVCodecsProvider* SysAppsManager::GetAVCodecsProvider() {
  CR_DEFINE_STATIC_LOCAL(AVCodecsProviderAndroid, provider, ());

  return &provider;
}

// static
StorageInfoProvider* SysAppsManager::GetStorageInfoProvider() {
  CR_DEFINE_STATIC_LOCAL(StorageInfoProviderAndroid, provider, ());

  return &provider;
}

}  // namespace sysapps
}  // namespace xwalk
