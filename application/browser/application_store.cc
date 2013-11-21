// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_store.h"

#include <utility>

#include "xwalk/application/common/application_file_util.h"
#include "xwalk/runtime/browser/runtime_context.h"

namespace xwalk {
namespace application {

const char ApplicationStore::kManifestPath[] = "manifest";

const char ApplicationStore::kApplicationPath[] = "path";

const char ApplicationStore::kInstallTime[] = "install_time";

const char ApplicationStore::kRegisteredEvents[] = "registered_events";

namespace {
inline const std::string GetRegisteredEventsPath(
    const std::string& application_id) {
  return application_id + "." + ApplicationStore::kRegisteredEvents;
}
}

ApplicationStore::ApplicationStore(xwalk::RuntimeContext* runtime_context)
    : runtime_context_(runtime_context),
      db_store_(new DBStoreImpl(runtime_context->GetPath())),
      applications_(new ApplicationMap) {
  db_store_->AddObserver(this);
  db_store_->InitDB();
}

ApplicationStore::~ApplicationStore() {
  db_store_->RemoveObserver(this);
}

bool ApplicationStore::AddApplication(
    scoped_refptr<const Application> application) {
  if (Contains(application->ID()))
    return true;

  if (!db_store_->Insert(application.get(), base::Time::Now()) ||
      !Insert(application))
    return false;

  return true;
}

bool ApplicationStore::RemoveApplication(const std::string& id) {
  if (applications_->erase(id) != 1) {
    LOG(ERROR) << "Application " << id << " is invalid.";
    return false;
  }

  if (!db_store_->Remove(id)) {
    LOG(ERROR) << "Error occurred while trying to remove application"
                  "information with id "
               << id << " from database.";
    return false;
  }
  return true;
}

bool ApplicationStore::Contains(const std::string& app_id) const {
  return applications_->find(app_id) != applications_->end();
}

scoped_refptr<const Application> ApplicationStore::GetApplicationByID(
    const std::string& application_id) const {
  ApplicationMapIterator it = applications_->find(application_id);
  if (it != applications_->end()) {
    return it->second;
  }

  return NULL;
}

ApplicationStore::ApplicationMap*
ApplicationStore::GetInstalledApplications() const {
  return applications_.get();
}

base::ListValue* ApplicationStore::GetApplicationEvents(const std::string& id) {
  base::DictionaryValue* db = db_store_->GetApplications();
  if (!db)
    return NULL;

  base::DictionaryValue* value = NULL;
  base::ListValue* events = NULL;
  if (!db->GetDictionary(id, &value) ||
      !value->GetList(ApplicationStore::kRegisteredEvents, &events))
    return NULL;
  return events;
}

bool ApplicationStore::SetApplicationEvents(
    const std::string& id,
    base::ListValue* events) {
  base::DictionaryValue* db = db_store_->GetApplications();
  if (!db || !db->HasKey(id)) {
    LOG(ERROR) << "Application " << id << " is not installed. "
               << "Could not set system events for it.";
    return false;
  }

  if (!events)
    return db_store_->Remove(GetRegisteredEventsPath(id));

  db_store_->SetValue(GetRegisteredEventsPath(id), events);
  return true;
}

void ApplicationStore::InitApplications(const base::DictionaryValue* db) {
  CHECK(db);

  for (base::DictionaryValue::Iterator it(*db); !it.IsAtEnd();
       it.Advance()) {
    const std::string& id = it.key();
    const base::DictionaryValue* value;
    const base::DictionaryValue* manifest;
    std::string app_path;
    if (!it.value().GetAsDictionary(&value) ||
        !value->GetString(ApplicationStore::kApplicationPath, &app_path) ||
        !value->GetDictionary(ApplicationStore::kManifestPath, &manifest))
      break;

    std::string error;
    scoped_refptr<Application> application =
        Application::Create(base::FilePath::FromUTF8Unsafe(app_path),
                            Manifest::INTERNAL,
                            *manifest,
                            id,
                            &error);
    if (!application) {
      LOG(ERROR) << "Load appliation error: " << error;
      break;
    }

    if (!Insert(application)) {
      LOG(ERROR) << "An error occurred while"
                    "initializing the application data.";
      break;
    }
  }
}

bool ApplicationStore::Insert(scoped_refptr<const Application> application) {
  return applications_->insert(
      std::pair<std::string, scoped_refptr<const Application> >(
          application->ID(), application)).second;
}

void ApplicationStore::OnDBValueChanged(const std::string& key,
                                        const base::Value* value) {
}

void ApplicationStore::OnInitializationCompleted(bool succeeded) {
  if (succeeded)
    InitApplications(db_store_->GetApplications());
}

}  // namespace application
}  // namespace xwalk
