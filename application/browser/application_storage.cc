// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_storage.h"
#include "xwalk/application/browser/application_storage_impl.h"

namespace xwalk {
namespace application {

ApplicationStorage::ApplicationStorage(const base::FilePath& path)
    : impl_(new ApplicationStorageImpl(path)) {
  impl_->Init();
}

ApplicationStorage::~ApplicationStorage() {
}

bool ApplicationStorage::AddApplication(
    scoped_refptr<ApplicationData> app_data) {
  if (Contains(app_data->ID())) {
    LOG(WARNING) << "Application " << app_data->ID()
                 << " has been already installed";
    return false;
  }

  return impl_->AddApplication(app_data.get(), base::Time::Now());
}

bool ApplicationStorage::RemoveApplication(const std::string& id) {
  return impl_->RemoveApplication(id);
}

bool ApplicationStorage::UpdateApplication(
    scoped_refptr<ApplicationData> app_data) {
  return impl_->UpdateApplication(app_data.get(), base::Time::Now());
}

bool ApplicationStorage::Contains(const std::string& app_id) const {
  return impl_->ContainsApplication(app_id);
}

scoped_refptr<ApplicationData> ApplicationStorage::GetApplicationData(
    const std::string& app_id) const {
  return impl_->GetApplicationData(app_id);
}

bool ApplicationStorage::GetInstalledApplications(
    ApplicationData::ApplicationDataMap& apps) const { // NOLINT
  return impl_->GetInstalledApplications(apps);
}

}  // namespace application
}  // namespace xwalk
