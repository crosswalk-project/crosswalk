// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/sysapps_manager.h"
#include "xwalk/sysapps/raw_socket/raw_socket_extension.h"

namespace xwalk {
namespace sysapps {

SysAppsManager::SysAppsManager()
    : raw_sockets_enabled_(true) {}

SysAppsManager::~SysAppsManager() {}

void SysAppsManager::DisableRawSockets() {
  raw_sockets_enabled_ = false;
}

void SysAppsManager::CreateExtensionsForUIThread(
    XWalkExtensionVector* extensions) {
    // This method was used to create device capability extension which
    // was removed, this method is reserved for furture use.
}

void SysAppsManager::CreateExtensionsForExtensionThread(
    XWalkExtensionVector* extensions) {
  if (raw_sockets_enabled_)
    extensions->push_back(new RawSocketExtension());
}

}  // namespace sysapps
}  // namespace xwalk
