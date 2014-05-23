// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_storage.h"

#include <utility>

#include "xwalk/application/browser/application_storage_impl.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/runtime/browser/runtime_context.h"

namespace xwalk {
namespace application {

ApplicationStorage::ApplicationStorage(const base::FilePath& path)
    : data_path_(path),
      impl_(new ApplicationStorageImpl(path)) {
  impl_->Init(applications_);
}

ApplicationStorage::~ApplicationStorage() {
}

bool ApplicationStorage::AddApplication(
    scoped_refptr<ApplicationData> app_data) {
  base::AutoLock lock(lock_);
  if (applications_.find(app_data->ID()) != applications_.end()) {
    LOG(WARNING) << "Application " << app_data->ID()
                 << " has been already installed";
    return false;
  }

  if (!impl_->AddApplication(app_data.get(), base::Time::Now()) ||
      !Insert(app_data))
    return false;

  return true;
}

bool ApplicationStorage::RemoveApplication(const std::string& id) {
  base::AutoLock lock(lock_);
  if (applications_.erase(id) != 1) {
    LOG(ERROR) << "Application " << id << " is invalid.";
    return false;
  }

  if (!impl_->RemoveApplication(id)) {
    LOG(ERROR) << "Error occurred while trying to remove application"
                  "information with id "
               << id << " from database.";
    return false;
  }

  return true;
}

bool ApplicationStorage::UpdateApplication(
    scoped_refptr<ApplicationData> app_data) {
  base::AutoLock lock(lock_);
  ApplicationData::ApplicationDataMapIterator it =
      applications_.find(app_data->ID());
  if (it == applications_.end()) {
    LOG(ERROR) << "Application " << app_data->ID() << " is invalid.";
    return false;
  }

  it->second = app_data;
  if (!impl_->UpdateApplication(app_data.get(), base::Time::Now()))
    return false;

  return true;
}

bool ApplicationStorage::Contains(const std::string& app_id) const {
  base::AutoLock lock(lock_);
  return applications_.find(app_id) != applications_.end();
}

scoped_refptr<ApplicationData> ApplicationStorage::GetApplicationData(
    const std::string& application_id) const {
  base::AutoLock lock(lock_);
  ApplicationData::ApplicationDataMap::const_iterator it =
      applications_.find(application_id);
  if (it != applications_.end()) {
    return it->second;
  }

  return NULL;
}

const ApplicationData::ApplicationDataMap&
ApplicationStorage::GetInstalledApplications() const {
  return applications_;
}

bool ApplicationStorage::Insert(scoped_refptr<ApplicationData> app_data) {
  return applications_.insert(
      std::pair<std::string, scoped_refptr<ApplicationData> >(
          app_data->ID(), app_data)).second;
}

}  // namespace application
}  // namespace xwalk
