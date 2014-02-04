// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_STORAGE_INFO_PROVIDER_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_STORAGE_INFO_PROVIDER_H_

#include <vector>
#include "base/callback_forward.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities.h"

namespace xwalk {
namespace sysapps {

using jsapi::device_capabilities::StorageUnit;
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

  class Observer {
   public:
    Observer() {}
    virtual ~Observer() {}

    virtual void OnStorageAttached(const StorageUnit& storage) = 0;
    virtual void OnStorageDetached(const StorageUnit& storage) = 0;
  };

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);
  bool HasObserver(Observer* observer) const;

 protected:
  StorageInfoProvider();

  virtual void StartStorageMonitoring() = 0;
  virtual void StopStorageMonitoring() = 0;

  void NotifyStorageAttached(const StorageUnit& storage);
  void NotifyStorageDetached(const StorageUnit& storage);

 private:
  bool is_initialized_;
  std::vector<base::Closure> callbacks_;
  ObserverList<Observer> observer_list_;

  DISALLOW_COPY_AND_ASSIGN(StorageInfoProvider);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_STORAGE_INFO_PROVIDER_H_
