// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/storage_info_provider.h"

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

void StorageInfoProvider::AddObserver(Observer* observer) {
  bool should_start_monitoring = false;
  if (!observer_list_.might_have_observers())
    should_start_monitoring = true;

  observer_list_.AddObserver(observer);

  // We start monitoring only when the first observer is added. Unfortunately
  // there is no public size() function for the observer list, so we have to
  // query for emptiness and store the status. StartStorageMonitoring() is
  // called after adding the observer so it doesn't miss any notification.
  if (should_start_monitoring)
    StartStorageMonitoring();
}

void StorageInfoProvider::RemoveObserver(Observer* observer) {
  observer_list_.RemoveObserver(observer);

  if (!observer_list_.might_have_observers())
    StopStorageMonitoring();
}

bool StorageInfoProvider::HasObserver(Observer* observer) const {
  return observer_list_.HasObserver(observer);
}

void StorageInfoProvider::NotifyStorageAttached(const StorageUnit& storage) {
  FOR_EACH_OBSERVER(Observer, observer_list_, OnStorageAttached(storage));
}

void StorageInfoProvider::NotifyStorageDetached(const StorageUnit& storage) {
  FOR_EACH_OBSERVER(Observer, observer_list_, OnStorageDetached(storage));
}

}  // namespace sysapps
}  // namespace xwalk
