// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_storage_impl.h"

#include <string>
#include <vector>

#include "base/file_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/json/json_file_value_serializer.h"
#include "base/json/json_string_value_serializer.h"
#include "sql/statement.h"
#include "sql/transaction.h"
#include "xwalk/application/browser/application_storage.h"
#include "xwalk/application/common/application_storage_constants.h"

namespace db_fields = xwalk::application_storage_constants;
namespace xwalk {
namespace application {

const base::FilePath::CharType ApplicationStorageImpl::kDBFileName[] =
    FILE_PATH_LITERAL("applications.db");

const char kEventSeparator = ';';

// Switching the JSON format DB(version 0) to SQLite backend version 1,
// should migrate all data from JSON DB to SQLite applications table.
static const int kVersionNumber = 1;

namespace {

inline const base::FilePath GetDBPath(const base::FilePath& path) {
  return path.Append(ApplicationStorageImpl::kDBFileName);
}

// Initializes the applications table, returning true on success.
bool InitApplicationsTable(sql::Connection* db) {
  sql::Transaction transaction(db);
  transaction.Begin();
  // The table is named "applications", the primary key is "id".
  if (!db->DoesTableExist(db_fields::kAppTableName)) {
    if (!db->Execute(db_fields::kCreateAppTableOp))
      return false;
  }
  return transaction.Commit();
}

bool InitEventsTable(sql::Connection* db) {
  sql::Transaction transaction(db);
  transaction.Begin();
  if (!db->DoesTableExist(db_fields::kEventTableName)) {
    if (!db->Execute(db_fields::kCreateEventTableOp))
     return false;
  }
  return transaction.Commit();
}

bool Insert(scoped_refptr<ApplicationData> application,
            ApplicationData::ApplicationDataMap& applications) {
  return applications.insert(
      std::pair<std::string, scoped_refptr<ApplicationData> >(
          application->ID(), application)).second;
}

}  // namespace

ApplicationStorageImpl::ApplicationStorageImpl(const base::FilePath& path)
    : data_path_(path),
      db_initialized_(false) {
  // Ensure the parent directory for database file is created before reading
  // from it.
  if (!base::PathExists(path) && !file_util::CreateDirectory(path))
    return;
}

bool ApplicationStorageImpl::UpgradeToVersion1(const base::FilePath& v0_file) {
  JSONFileValueSerializer serializer(v0_file);
  int error_code;
  std::string error;
  base::Value* old_db = serializer.Deserialize(&error_code, &error);
  if (!old_db) {
    LOG(ERROR) << "Unable to read applications information from JSON DB, "
                  "the error message is: "
               << error;
    return false;
  }

  scoped_ptr<base::DictionaryValue> value;
  for (base::DictionaryValue::Iterator it(
           *static_cast<base::DictionaryValue*>(old_db));
       !it.IsAtEnd(); it.Advance()) {
    value.reset(static_cast<base::DictionaryValue*>(it.value().DeepCopy()));
    base::DictionaryValue* manifest;
    value->GetDictionary("manifest", &manifest);
    std::string path;
    value->GetString("path", &path);
    std::string error;
    scoped_refptr<ApplicationData> application =
        ApplicationData::Create(base::FilePath::FromUTF8Unsafe(path),
                                Manifest::INTERNAL,
                                *manifest,
                                it.key(),
                                &error);
    if (!application) {
      LOG(ERROR) << "Unable to migrate the information to Version 1: " << error;
      return false;
    }

    double install_time;
    value->GetDouble("install_time", &install_time);
    if (!AddApplication(application, base::Time::FromDoubleT(install_time)))
      return false;
  }
  meta_table_.SetVersionNumber(1);

  return true;
}

ApplicationStorageImpl::~ApplicationStorageImpl() {
}

bool ApplicationStorageImpl::Init(
    ApplicationData::ApplicationDataMap& applications) {
  bool does_db_exist = base::PathExists(GetDBPath(data_path_));
  scoped_ptr<sql::Connection> sqlite_db(new sql::Connection);
  if (!sqlite_db->Open(GetDBPath(data_path_))) {
    LOG(ERROR) << "Unable to open applications DB.";
    return false;
  }
  sqlite_db->Preload();

  if (!meta_table_.Init(sqlite_db.get(), kVersionNumber, kVersionNumber) ||
      meta_table_.GetVersionNumber() != kVersionNumber) {
    LOG(ERROR) << "Unable to init the META table.";
    return false;
  }

  if (!InitApplicationsTable(sqlite_db.get())) {
    LOG(ERROR) << "Unable to open applications table.";
    return false;
  }

  if (!InitEventsTable(sqlite_db.get())) {
    LOG(ERROR) << "Unable to open registered events table.";
    return false;
  }

  if (!sqlite_db->Execute("PRAGMA foreign_keys=ON")) {
    LOG(ERROR) << "Unable to enforce foreign key contraints.";
    return false;
  }

  sqlite_db_.reset(sqlite_db.release());

  db_initialized_ = (sqlite_db_ && sqlite_db_->is_open());

  base::FilePath v0_file = data_path_.Append(
      FILE_PATH_LITERAL("applications_db"));
  if (base::PathExists(v0_file) &&
      !does_db_exist) {
    if (!UpgradeToVersion1(v0_file)) {
      LOG(ERROR) << "Unable to migrate database from JSON format to SQLite.";
      return false;
    }
    // After migrated to SQLite, delete the old JSON DB file is safe,
    // since all information has been migrated and it will not be used anymore.
    if (!base::DeleteFile(v0_file, false)) {
      LOG(ERROR) << "Unalbe to delete old JSON DB file.";
      return false;
    }
  }

  db_initialized_ = GetInstalledApplications(applications);

  return db_initialized_;
}

bool ApplicationStorageImpl::GetInstalledApplications(
    ApplicationData::ApplicationDataMap& applications) {
  if (!db_initialized_) {
    LOG(ERROR) << "The database haven't initilized.";
    return false;
  }

  sql::Statement smt(sqlite_db_->GetUniqueStatement(
      db_fields::kGetAllRowsFromAppEventTableOp));
  if (!smt.is_valid())
    return false;

  std::string error_msg;
  while (smt.Step()) {
    std::string id = smt.ColumnString(0);

    int error_code;
    std::string manifest_str = smt.ColumnString(1);
    JSONStringValueSerializer serializer(&manifest_str);
    base::Value* manifest = serializer.Deserialize(&error_code, &error_msg);
    if (manifest == NULL) {
      LOG(ERROR) << "An error occured when deserializing the manifest, "
                    "the error message is: "
                 << error_msg;
      return false;
    }
    std::string path = smt.ColumnString(2);
    double install_time = smt.ColumnDouble(3);
    std::vector<std::string> events;
    base::SplitString(smt.ColumnString(4), kEventSeparator, &events);

    std::string error;
    scoped_refptr<ApplicationData> application =
        ApplicationData::Create(
            base::FilePath::FromUTF8Unsafe(path),
            Manifest::INTERNAL,
            *(static_cast<base::DictionaryValue*>(manifest)),
            id,
            &error);
    if (!application) {
      LOG(ERROR) << "Load appliation error: " << error;
      return false;
    }

    application->install_time_ = base::Time::FromDoubleT(install_time);

    if (!events.empty()) {
      application->events_ =
          std::set<std::string>(events.begin(), events.end());
    }

    if (!Insert(application, applications)) {
      LOG(ERROR) << "An error occurred while"
                    "initializing the application cache data.";
      return false;
    }
  }

  return true;
}

bool ApplicationStorageImpl::AddApplication(const ApplicationData* application,
                                            const base::Time& install_time) {
  if (!db_initialized_) {
    LOG(ERROR) << "The database haven't initialized.";
    return false;
  }

  return (SetApplicationValue(
      application, install_time, db_fields::kSetApplicationWithBindOp) &&
          SetEvents(application->ID(), application->GetEvents()));
}

bool ApplicationStorageImpl::UpdateApplication(
    ApplicationData* application, const base::Time& install_time) {
  if (!db_initialized_) {
    LOG(ERROR) << "The database haven't initialized.";
    return false;
  }

  if (SetApplicationValue(
          application, install_time, db_fields::kUpdateApplicationWithBindOp) &&
      UpdateEvents(application->ID(), application->GetEvents())) {
    application->is_dirty_ = false;
    return true;
  }

  return false;
}

bool ApplicationStorageImpl::RemoveApplication(const std::string& id) {
  if (!db_initialized_) {
    LOG(ERROR) << "The database haven't initialized.";
    return false;
  }

  sql::Transaction transaction(sqlite_db_.get());
  if (!transaction.Begin())
    return false;

  sql::Statement smt(sqlite_db_->GetUniqueStatement(
      db_fields::kDeleteApplicationWithBindOp));
  smt.BindString(0, id);
  if (!smt.Run()) {
    LOG(ERROR) << "Could not delete application "
                  "information from DB.";
    return false;
  }

  return transaction.Commit();
}

bool ApplicationStorageImpl::SetApplicationValue(
    const ApplicationData* application,
    const base::Time& install_time,
    const std::string& operation) {
  if (!application) {
    LOG(ERROR) << "A value is needed when inserting/updating in DB.";
    return false;
  }

  std::string manifest;
  JSONStringValueSerializer serializer(&manifest);
  if (!serializer.Serialize(*(application->GetManifest()->value()))) {
    LOG(ERROR) << "An error occured when serializing the manifest value.";
    return false;
  }

  sql::Transaction transaction(sqlite_db_.get());
  if (!transaction.Begin())
    return false;

  sql::Statement smt(sqlite_db_->GetUniqueStatement(operation.c_str()));
  if (!smt.is_valid()) {
    LOG(ERROR) << "Unable to insert/update application info in DB.";
    return false;
  }
  smt.BindString(0, manifest);
  smt.BindString(1, application->Path().AsUTF8Unsafe());
  smt.BindDouble(2, install_time.ToDoubleT());
  smt.BindString(3, application->ID());
  if (!smt.Run()) {
    LOG(ERROR) << "An error occured when inserting/updating "
                  "application info in DB.";
    return false;
  }

  return transaction.Commit();
}

bool ApplicationStorageImpl::SetEvents(const std::string& id,
                                       const std::set<std::string>& events) {
  if (!db_initialized_)
    return false;
  return SetEventsValue(id, events, db_fields::kInsertEventsWithBindOp);
}

bool ApplicationStorageImpl::UpdateEvents(
    const std::string &id, const std::set<std::string>& events) {
  if (!db_initialized_)
    return false;

  if (events.empty())
    return DeleteEvents(id);

  return SetEventsValue(id, events, db_fields::kUpdateEventsWithBindOp);
}

bool ApplicationStorageImpl::DeleteEvents(const std::string& id) {
  sql::Transaction transaction(sqlite_db_.get());
  if (!transaction.Begin())
    return false;

  sql::Statement smt(sqlite_db_->GetUniqueStatement(
      db_fields::kDeleteEventsWithBindOp));
  smt.BindString(0, id);

  if (!smt.Run()) {
    LOG(ERROR) << "An error occured when deleting event information from DB.";
    return false;
  }

  return transaction.Commit();
}

bool ApplicationStorageImpl::SetEventsValue(
    const std::string& id,
    const std::set<std::string>& events,
    const std::string& operation) {
  sql::Transaction transaction(sqlite_db_.get());
  std::string events_list(JoinString(
      std::vector<std::string>(events.begin(), events.end()), kEventSeparator));

  if (!transaction.Begin())
    return false;

  sql::Statement smt(sqlite_db_->GetUniqueStatement(
      operation.c_str()));
  smt.BindString(0, events_list);
  smt.BindString(1, id);
  if (!smt.Run()) {
    LOG(ERROR) << "An error occured when inserting event information into DB.";
    return false;
  }

  return transaction.Commit();
}

}  // namespace application
}  // namespace xwalk
