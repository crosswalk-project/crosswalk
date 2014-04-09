// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/storage_info_provider_chromium.h"

#include <string>
#include <vector>

#include "base/bind_helpers.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "components/storage_monitor/storage_monitor.h"

using storage_monitor::StorageInfo;
using storage_monitor::StorageMonitor;
using namespace xwalk::jsapi::device_capabilities; // NOLINT

namespace {

linked_ptr<StorageUnit> makeStorageUnit(const StorageInfo& storage) {
  linked_ptr<StorageUnit> storage_unit(make_linked_ptr(new StorageUnit));

  storage_unit->id = storage.device_id();
  storage_unit->name = base::UTF16ToUTF8(storage.GetDisplayName(false));
  storage_unit->capacity = storage.total_size_in_bytes();

  if (StorageInfo::IsRemovableDevice(storage_unit->id))
    storage_unit->type = ToString(STORAGE_UNIT_TYPE_REMOVABLE);
  else if (StorageInfo::IsMassStorageDevice(storage_unit->id))
    storage_unit->type = ToString(STORAGE_UNIT_TYPE_FIXED);
  else
    storage_unit->type = ToString(STORAGE_UNIT_TYPE_UNKNOWN);

  return storage_unit;
}

}  // namespace

namespace xwalk {
namespace sysapps {

StorageInfoProviderChromium::StorageInfoProviderChromium() {
  if (!StorageMonitor::GetInstance())
    StorageMonitor::Create();

  StorageMonitor* monitor = StorageMonitor::GetInstance();

  monitor->EnsureInitialized(
      base::Bind(&StorageInfoProvider::MarkInitialized,
                 base::Unretained(this)));
}

StorageInfoProviderChromium::~StorageInfoProviderChromium() {}

scoped_ptr<SystemStorage> StorageInfoProviderChromium::storage_info() const {
  scoped_ptr<SystemStorage> info(new SystemStorage);

  StorageMonitor* monitor = StorageMonitor::GetInstance();
  DCHECK(monitor->IsInitialized());

  std::vector<StorageInfo> storages = monitor->GetAllAvailableStorages();
  for (std::vector<StorageInfo>::const_iterator it = storages.begin();
      it != storages.end(); ++it) {
    info->storages.push_back(makeStorageUnit(*it));
  }

  return info.Pass();
}

void StorageInfoProviderChromium::OnRemovableStorageAttached(
    const StorageInfo& info) {
  NotifyStorageAttached(*makeStorageUnit(info));
}

void StorageInfoProviderChromium::OnRemovableStorageDetached(
    const StorageInfo& info) {
  NotifyStorageDetached(*makeStorageUnit(info));
}

void StorageInfoProviderChromium::StartStorageMonitoring() {
  StorageMonitor::GetInstance()->AddObserver(this);
}

void StorageInfoProviderChromium::StopStorageMonitoring() {
  StorageMonitor::GetInstance()->RemoveObserver(this);
}

}  // namespace sysapps
}  // namespace xwalk
