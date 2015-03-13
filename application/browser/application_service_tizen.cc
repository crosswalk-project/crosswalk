// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_service_tizen.h"

#include <algorithm>
#include <string>
#include <vector>

#include "xwalk/application/browser/application.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/common/tizen/application_storage.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"

namespace xwalk {

namespace application {

namespace {

const base::FilePath::CharType kApplicationDataDirName[] =
    FILE_PATH_LITERAL("Storage/ext");

base::FilePath GetStoragePartitionPath(
    const base::FilePath& base_path, std::string app_id) {
  std::transform(app_id.begin(), app_id.end(), app_id.begin(), ::tolower);
  return base_path.Append(kApplicationDataDirName).Append(app_id);
}

void CollectUnusedStoragePartitions(XWalkBrowserContext* context,
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
    XWalkBrowserContext* browser_context)
  : ApplicationService(browser_context),
    application_storage_(new ApplicationStorage(browser_context->GetPath())) {
  CollectUnusedStoragePartitions(browser_context, application_storage_.get());
}

ApplicationServiceTizen::~ApplicationServiceTizen() {
}

Application* ApplicationServiceTizen::LaunchFromAppID(
    const std::string& id, const std::string& encoded_bundle) {
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

  // Set bundle data for app
  app_data->set_bundle(encoded_bundle);

  return Launch(app_data);
}

}  // namespace application
}  // namespace xwalk
