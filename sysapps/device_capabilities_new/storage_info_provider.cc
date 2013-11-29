// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities_new/storage_info_provider.h"

#include "base/callback.h"

namespace xwalk {
namespace sysapps {

StorageInfoProvider::StorageInfoProvider()
  : is_initialized_(false) {}

StorageInfoProvider::~StorageInfoProvider() {}

void StorageInfoProvider::AddOnInitCallback(base::Closure callback) {
  DCHECK(!is_initialized_);
  callbacks_.push_back(callback);
}

void StorageInfoProvider::MarkInitialized() {
  DCHECK(!is_initialized_);
  is_initialized_ = true;

  for (std::vector<base::Closure>::const_iterator it = callbacks_.begin();
      it != callbacks_.end(); ++it) {
    it->Run();
  }

  callbacks_.clear();
}

}  // namespace sysapps
}  // namespace xwalk
