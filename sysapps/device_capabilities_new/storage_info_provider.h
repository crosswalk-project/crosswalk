// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_STORAGE_INFO_PROVIDER_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_STORAGE_INFO_PROVIDER_H_

#include <vector>
#include "base/callback_forward.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/sysapps/device_capabilities_new/device_capabilities.h"

namespace xwalk {
namespace sysapps {

using jsapi::device_capabilities::SystemStorage;

class StorageInfoProvider {
 public:
  virtual ~StorageInfoProvider();

  // The storage backend might not be ready yet when the instance is created. In
  // this case, we can use this method to queue tasks.
  void AddOnInitCallback(base::Closure callback);

  bool IsInitialized() const { return is_initialized_; }
  void MarkInitialized();

  virtual scoped_ptr<SystemStorage> storage_info() const = 0;

 protected:
  StorageInfoProvider();

 private:
  bool is_initialized_;
  std::vector<base::Closure> callbacks_;

  DISALLOW_COPY_AND_ASSIGN(StorageInfoProvider);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_NEW_STORAGE_INFO_PROVIDER_H_
