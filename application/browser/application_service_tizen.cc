// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_service_tizen.h"

#include <string>
#include <vector>

#include "xwalk/application/browser/application.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/common/tizen/application_storage.h"
#include "xwalk/runtime/browser/runtime_context.h"

namespace xwalk {

namespace application {

namespace {

const base::FilePath::CharType kApplicationDataDirName[] =
    FILE_PATH_LITERAL("Storage/ext");

base::FilePath GetStoragePartitionPath(
    const base::FilePath& base_path, const std::string& app_id) {
  return base_path.Append(kApplicationDataDirName).Append(app_id);
}

void CollectUnusedStoragePartitions(RuntimeContext* context,
                                    ApplicationStorage* storage) {
  std::vector<std::string> app_ids;
  if (!storage->GetInstalledApplicationIDs(app_ids))
    return;

  scoped_ptr<base::hash_set<base::FilePath> > active_paths(
      new base::hash_set<base::FilePath>()); // NOLINT

  for (unsigned i = 0; i < app_ids.size(); ++i) {
    active_paths->insert(
        GetStoragePartitionPath(context->GetPath(), app_ids.at(i)));
  }

  content::BrowserContext::GarbageCollectStoragePartitions(
      context, active_paths.Pass(), base::Bind(&base::DoNothing));
}

}  // namespace

ApplicationServiceTizen::ApplicationServiceTizen(
    RuntimeContext* runtime_context)
  : ApplicationService(runtime_context),
    application_storage_(new ApplicationStorage(runtime_context->GetPath())) {
  CollectUnusedStoragePartitions(runtime_context, application_storage_.get());
}

ApplicationServiceTizen::~ApplicationServiceTizen() {
}

Application* ApplicationServiceTizen::LaunchFromAppID(
    const std::string& id, const Application::LaunchParams& params) {
  if (!IsValidApplicationID(id)) {
     LOG(ERROR) << "The input parameter is not a valid app id: " << id;
     return NULL;
  }

  scoped_refptr<ApplicationData> app_data =
    application_storage_->GetApplicationData(id);
  if (!app_data.get()) {
    LOG(ERROR) << "Application with id " << id << " is not installed.";
    return NULL;
  }

  return Launch(app_data, params);
}

}  // namespace application
}  // namespace xwalk
