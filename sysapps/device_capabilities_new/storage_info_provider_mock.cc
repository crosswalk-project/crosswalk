// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities_new/storage_info_provider_mock.h"

#include <string>

using xwalk::jsapi::device_capabilities::StorageUnit;

namespace {

const char kRemovableStorageName[] = "Removable";

linked_ptr<StorageUnit> makeStorageUnit(const std::string& name) {
  linked_ptr<StorageUnit> storage(make_linked_ptr(new StorageUnit));

  storage->id = "ID " + name;
  storage->name = "Name " + name;
  storage->type = "Type " + name;
  storage->capacity = 100000;

  return storage;
}

}  // namespace

namespace xwalk {
namespace sysapps {

StorageInfoProviderMock::StorageInfoProviderMock()
  : is_removable_storage_present(false) {
  MarkInitialized();
}

StorageInfoProviderMock::~StorageInfoProviderMock() {}

scoped_ptr<SystemStorage> StorageInfoProviderMock::storage_info() const {
  scoped_ptr<SystemStorage> info(new SystemStorage);

  info->storages.push_back(makeStorageUnit("1"));
  info->storages.push_back(makeStorageUnit("2"));
  info->storages.push_back(makeStorageUnit("3"));

  if (is_removable_storage_present)
    info->storages.push_back(makeStorageUnit(kRemovableStorageName));

  return info.Pass();
}

void StorageInfoProviderMock::StartStorageMonitoring() {
  timer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(1),
      this, &StorageInfoProviderMock::MockStorageEvent);
}

void StorageInfoProviderMock::StopStorageMonitoring() {
  timer_.Stop();
}

void StorageInfoProviderMock::MockStorageEvent() {
  is_removable_storage_present = !is_removable_storage_present;

  if (is_removable_storage_present)
    NotifyStorageAttached(*makeStorageUnit(kRemovableStorageName));
  else
    NotifyStorageDetached(*makeStorageUnit(kRemovableStorageName));
}

}  // namespace sysapps
}  // namespace xwalk
