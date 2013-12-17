// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/sysapps_manager.h"

#include "xwalk/runtime/common/xwalk_runtime_features.h"
#include "xwalk/sysapps/raw_socket/raw_socket_extension.h"

namespace xwalk {
namespace sysapps {

SysAppsManager::SysAppsManager() {}

SysAppsManager::~SysAppsManager() {}

void SysAppsManager::CreateExtensionsForUIThread(
    XWalkExtensionVector* extensions) {
}

void SysAppsManager::CreateExtensionsForExtensionThread(
    XWalkExtensionVector* extensions) {
  if (XWalkRuntimeFeatures::isRawSocketsAPIEnabled())
    extensions->push_back(new RawSocketExtension());
}

}  // namespace sysapps
}  // namespace xwalk
