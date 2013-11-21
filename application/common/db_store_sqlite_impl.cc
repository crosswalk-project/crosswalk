// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/db_store_sqlite_impl.h"

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
#include "xwalk/application/browser/application_store.h"
#include "xwalk/application/common/db_store_constants.h"

namespace db_fields = xwalk::db_store_constants;
namespace xwalk {
namespace application {

const base::FilePath::CharType DBStoreSqliteImpl::kDBFileName[] =
    FILE_PATH_LITERAL("applications.db");

const char kEventSeparator = ';';

// Switching the JSON format DB(version 0) to SQLite backend version 1,
// should migrate all data from JSON DB to SQLite applications table.
static const int kVersionNumber = 1;

namespace {

inline const base::FilePath GetDBPath(const base::FilePath& path) {
  return path.Append(DBStoreSqliteImpl::kDBFileName);
}

inline const std::string GetManifestPath(const std::string& application_id) {
  return application_id + "." + ApplicationStore::kManifestPath;
}

inline const std::string GetApplicationPath(const std::string& application_id) {
  return application_id + "." + ApplicationStore::kApplicationPath;
}

inline const std::string GetInstallTimePath(const std::string& application_id) {
  return application_id + "." + ApplicationStore::kInstallTime;
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

}  // namespace

DBStoreSqliteImpl::DBStoreSqliteImpl(const base::FilePath& path)
    : DBStore(path),
      sqlite_db_(new sql::Connection),
      db_initialized_(false) {
  // Ensure the parent directory for database file is created before reading
  // from it.
  if (!base::PathExists(path) && !file_util::CreateDirectory(path))
    return;

  bool does_db_exist = base::PathExists(GetDBPath(path));
  if (!sqlite_db_->Open(GetDBPath(path))) {
    LOG(ERROR) << "Unable to open applications DB.";
    sqlite_db_.reset();
    return;
  }

  if (!meta_table_.Init(sqlite_db_.get(), kVersionNumber, kVersionNumber) ||
      meta_table_.GetVersionNumber() != kVersionNumber) {
    LOG(ERROR) << "Unable to init the META table.";
    return;
  }

  if (!InitApplicationsTable(sqlite_db_.get())) {
    LOG(ERROR) << "Unable to open applications table.";
    sqlite_db_.reset();
    return;
  }

  if (!InitEventsTable(sqlite_db_.get())) {
    LOG(ERROR) << "Unable to open registered events table.";
    sqlite_db_.reset();
    return;
  }

  base::FilePath v0_file = path.Append(FILE_PATH_LITERAL("applications_db"));
  if (base::PathExists(v0_file) &&
      !does_db_exist) {
    if (!UpgradeToVersion1(v0_file)) {
      LOG(ERROR) << "Unable to migrate database from JSON format to SQLite.";
      return;
    }
    // After migrated to SQLite, delete the old JSON DB file is safe,
    // since all information has been migrated and it will not be used anymore.
    if (!base::DeleteFile(v0_file, false)) {
      LOG(ERROR) << "Unalbe to delete old JSON DB file.";
      return;
    }
  }

  int ret = sqlite_db_->Execute("PRAGMA foreign_keys=ON");
  if (!ret) {
    LOG(ERROR) << "Unable to enforce foreign key contraints.";
    return;
  }
  sqlite_db_->Preload();
}

bool DBStoreSqliteImpl::UpgradeToVersion1(const base::FilePath& v0_file) {
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

  scoped_ptr<base::Value> value;
  for (base::DictionaryValue::Iterator it(
           *static_cast<base::DictionaryValue*>(old_db));
       !it.IsAtEnd(); it.Advance()) {
    value.reset(it.value().DeepCopy());
    if (!SetApplication(it.key(), value.get()))
      return false;
  }
  meta_table_.SetVersionNumber(1);

  return true;
}

DBStoreSqliteImpl::~DBStoreSqliteImpl() {
  if (sqlite_db_.get())
    sqlite_db_.reset();
}

bool DBStoreSqliteImpl::InitDB() {
  db_initialized_ = UpdateDBCache();

  FOR_EACH_OBSERVER(DBStore::Observer,
                    observers_,
                    OnInitializationCompleted(db_initialized_));
  return db_initialized_;
}

bool DBStoreSqliteImpl::UpdateDBCache() {
  if (!sqlite_db_ || !sqlite_db_->is_open())
    return false;

  // Read all installed appliations information to db memory cache.
  db_.reset(new base::DictionaryValue);
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
    std::vector<std::string> events_vec;
    base::ListValue* events_list = new base::ListValue;
    base::SplitString(smt.ColumnString(4), kEventSeparator, &events_vec);
    events_list->AppendStrings(events_vec);

    base::DictionaryValue* value = new base::DictionaryValue;
    value->Set(ApplicationStore::kManifestPath, manifest);
    value->SetString(ApplicationStore::kApplicationPath, path);
    value->SetDouble(ApplicationStore::kInstallTime, install_time);
    if (events_list->GetSize())
      value->Set(ApplicationStore::kRegisteredEvents, events_list);
    db_->Set(id, value);
  }

  return true;
}

bool DBStoreSqliteImpl::Insert(const Application* application,
                               const base::Time install_time) {
  if (!db_initialized_)
    return false;

  std::string application_id = application->ID();
  if (!db_->HasKey(application_id)) {
    base::DictionaryValue* manifest =
        application->GetManifest()->value()->DeepCopy();
    scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue);
    value->Set(ApplicationStore::kManifestPath, manifest);
    value->SetString(ApplicationStore::kApplicationPath,
                     application->Path().value());
    value->SetDouble(ApplicationStore::kInstallTime, install_time.ToDoubleT());
    SetValue(application_id, value.release());
  }
  return true;
}

bool DBStoreSqliteImpl::Remove(const std::string& key) {
  if (!db_initialized_)
    return false;

  std::string current_path(key);
  size_t delimiter_position = current_path.find('.');
  std::string application_id(current_path, 0, delimiter_position);

  if (!db_->HasKey(key)) {
    LOG(ERROR) << "Database key " << key << " is invalid.";
    return false;
  }

  if (!db_->Remove(key, NULL)) {
    LOG(ERROR) << "Cannot remove the record " << key
               << " from database cache.";
    return false;
  }
  ReportValueChanged(key, NULL);
  std::string column;
  if (delimiter_position != std::string::npos)
    column = std::string(current_path, delimiter_position+1);
  return Commit(application_id, column, NULL, ACTION_DELETE);
}

void DBStoreSqliteImpl::SetValue(const std::string& key, base::Value* value) {
  if (!db_initialized_)
    return;

  std::string current_path(key);
  size_t delimiter_position = current_path.find('.');
  std::string application_id(current_path, 0, delimiter_position);

  scoped_ptr<base::Value> new_value(value);
  base::Value* old_value = NULL;
  db_->Get(key, &old_value);
  if (!old_value || !value->Equals(old_value)) {
    base::Value* changed_value = new_value.release();
    db_->Set(key, changed_value);
    ReportValueChanged(key, changed_value);
    // Update the record in sqlite database.
    std::string column;
    if (delimiter_position != std::string::npos)
      column = std::string(current_path, delimiter_position+1);
    Action action = !!old_value? ACTION_UPDATE:ACTION_INSERT;
    Commit(application_id, column, changed_value, action);
  }
}

bool DBStoreSqliteImpl::SetApplication(
    const std::string& id, base::Value* value) {
  if (!value) {
    LOG(ERROR) << "A value is needed when inserting into DB.";
    return false;
  }

  std::string manifest;
  JSONStringValueSerializer serializer(&manifest);
  base::Value* manifest_value;
  if (!static_cast<base::DictionaryValue*>(value)->Get(
          ApplicationStore::kManifestPath, &manifest_value) ||
      !serializer.Serialize(*manifest_value)) {
    LOG(ERROR) << "An error occured when serializing the manifest value.";
    return false;
  }

  std::string path;
  if (!static_cast<base::DictionaryValue*>(value)->GetString(
          ApplicationStore::kApplicationPath, &path)) {
    LOG(ERROR) << "An error occured when getting path information.";
    return false;
  }

  double install_time;
  if (!static_cast<base::DictionaryValue*>(value)->GetDouble(
          ApplicationStore::kInstallTime, &install_time)) {
    LOG(ERROR) << "An error occured when getting install time information.";
    return false;
  }

  sql::Transaction transaction(sqlite_db_.get());
  if (!transaction.Begin())
    return false;

  sql::Statement smt(sqlite_db_->GetUniqueStatement(
      db_fields::kSetApplicationWithBindOp));
  if (!smt.is_valid()) {
    LOG(ERROR) << "Unable to insert application info into DB.";
    return false;
  }
  smt.BindString(0, manifest);
  smt.BindString(1, path);
  smt.BindDouble(2, install_time);
  smt.BindString(3, id);
  if (!smt.Run()) {
    LOG(ERROR) << "An error occured when inserting application info into DB.";
    return false;
  }

  return transaction.Commit();
}

bool DBStoreSqliteImpl::UpdateApplication(
    const std::string& id, base::Value* value) {
  if (!value) {
    LOG(ERROR) << "A value is needed when updating in DB.";
    return false;
  }

  std::string manifest;
  JSONStringValueSerializer serializer(&manifest);
  base::Value* manifest_value;
  if (!static_cast<base::DictionaryValue*>(value)->Get(
          ApplicationStore::kManifestPath, &manifest_value) ||
      !serializer.Serialize(*manifest_value)) {
    LOG(ERROR) << "An error occured when serializing the manifest value.";
    return false;
  }

  std::string path;
  if (!static_cast<base::DictionaryValue*>(value)->GetString(
          ApplicationStore::kApplicationPath, &path)) {
    LOG(ERROR) << "An error occured when getting path information.";
    return false;
  }

  double install_time;
  if (!static_cast<base::DictionaryValue*>(value)->GetDouble(
          ApplicationStore::kInstallTime, &install_time)) {
    LOG(ERROR) << "An error occured when getting install time information.";
    return false;
  }

  sql::Transaction transaction(sqlite_db_.get());
  if (!transaction.Begin())
    return false;

  sql::Statement smt(sqlite_db_->GetUniqueStatement(
      db_fields::kUpdateApplicationWithBindOp));
  if (!smt.is_valid()) {
    LOG(ERROR) << "Unable to update application info in DB.";
    return false;
  }
  smt.BindString(0, manifest);
  smt.BindString(1, path);
  smt.BindDouble(2, install_time);
  smt.BindString(3, id);
  if (!smt.Run()) {
    LOG(ERROR) << "An error occured when updating application info in DB.";
    return false;
  }

  return transaction.Commit();
}

bool DBStoreSqliteImpl::DeleteApplication(const std::string& id) {
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

bool DBStoreSqliteImpl::SetManifestValue(
    const std::string& id, base::Value* value) {
  if (!value) {
    LOG(ERROR) << "A value is needed when updating in DB.";
    return false;
  }

  std::string manifest;
  JSONStringValueSerializer serializer(&manifest);
  if (!serializer.Serialize(*value)) {
    LOG(ERROR) << "An error occured when serializing the manifest value.";
    return false;
  }

  sql::Transaction transaction(sqlite_db_.get());
  if (!transaction.Begin())
    return false;

  sql::Statement smt(sqlite_db_->GetUniqueStatement(
      db_fields::kSetManifestWithBindOp));
  if (!smt.is_valid()) {
    LOG(ERROR) << "Unable to update manifest in db.";
    return false;
  }
  smt.BindString(0, manifest);
  smt.BindString(1, id);
  if (!smt.Run()) {
    LOG(ERROR) << "Could not update application manifest "
                  "information in DB.";
    return false;
  }

  return transaction.Commit();
}

bool DBStoreSqliteImpl::SetInstallTimeValue(
    const std::string& id, base::Value* value) {
  if (!value) {
    LOG(ERROR) << "A value is needed when updating in DB.";
    return false;
  }

  double install_time;
  if (!static_cast<base::FundamentalValue*>(value)->GetAsDouble(
          &install_time)) {
    LOG(ERROR) << "An error occured when getting install time information.";
    return false;
  }

  sql::Transaction transaction(sqlite_db_.get());
  if (!transaction.Begin())
    return false;

  sql::Statement smt(sqlite_db_->GetUniqueStatement(
      db_fields::kSetInstallTimeWithBindOp));
  if (!smt.is_valid()) {
    LOG(ERROR) << "Unable to update install_time in db.";
    return false;
  }
  smt.BindDouble(0, install_time);
  smt.BindString(1, id);
  if (!smt.Run()) {
    LOG(ERROR) << "Could not update application install time "
                  "information in the DB.";
    return false;
  }

  return transaction.Commit();
}

bool DBStoreSqliteImpl::SetApplicationPathValue(
    const std::string& id, base::Value* value) {
  if (!value) {
    LOG(ERROR) << "A value is needed when updating in DB.";
    return false;
  }

  std::string path;
  if (!static_cast<base::StringValue*>(value)->GetAsString(&path)) {
    LOG(ERROR) << "An error occured when getting path information.";
    return false;
  }

  sql::Transaction transaction(sqlite_db_.get());
  if (!transaction.Begin())
    return false;

  sql::Statement smt(sqlite_db_->GetUniqueStatement(
      db_fields::kSetApplicationPathWithBindOp));
  if (!smt.is_valid()) {
    LOG(ERROR) << "Unable to update path in db.";
    return false;
  }
  smt.BindString(0, path);
  smt.BindString(1, id);
  if (!smt.Run()) {
    LOG(ERROR) << "Could not update application path information.";
    return false;
  }

  return transaction.Commit();
}

bool DBStoreSqliteImpl::Commit(const std::string& id,
                               const std::string& column,
                               base::Value* value,
                               Action action) {
  bool ret = false;
  switch (action) {
    case ACTION_INSERT:
      if (column.empty())
        ret = SetApplication(id, value);
      else if (column.compare(ApplicationStore::kRegisteredEvents) == 0)
        ret = SetEventsValue(id, value,
            std::string(db_fields::kInsertEventsWithBindOp));
      break;
    case ACTION_UPDATE:
      if (column.empty())
        ret = UpdateApplication(id, value);
      else if (column.compare(ApplicationStore::kManifestPath) == 0)
        ret = SetManifestValue(id, value);
      else if (column.compare(ApplicationStore::kApplicationPath) == 0)
        ret = SetApplicationPathValue(id, value);
      else if (column.compare(ApplicationStore::kInstallTime) == 0)
        ret = SetInstallTimeValue(id, value);
      else if (column.compare(ApplicationStore::kRegisteredEvents) == 0)
        ret = SetEventsValue(id, value,
            std::string(db_fields::kUpdateEventsWithBindOp));
      else
        LOG(ERROR) << "Unable to update the column "
                   << column << " in database.";
      break;
    case ACTION_DELETE:
      if (column.empty())
        ret = DeleteApplication(id);
      else if (column.compare(ApplicationStore::kRegisteredEvents) == 0)
        ret = DeleteEventsValue(id);
      break;
    case ACTION_UNKNOWN:
    default:
      LOG(ERROR) << "An unknow or illegal action has been committed.";
  }
  return ret;
}

bool DBStoreSqliteImpl::SetEventsValue(
    const std::string& id,
    base::Value* value,
    const std::string& operation) {
  if (!value)
    return false;

  base::ListValue* events = static_cast<base::ListValue*>(value);
  sql::Transaction transaction(sqlite_db_.get());

  std::vector<std::string> events_vec;
  for (size_t i = 0; i < events->GetSize(); ++i) {
    std::string event;
    if (events->GetString(i, &event))
      events_vec.push_back(event);
  }
  std::string events_list(JoinString(events_vec, kEventSeparator));

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

bool DBStoreSqliteImpl::DeleteEventsValue(
    const std::string& id) {
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

void DBStoreSqliteImpl::ReportValueChanged(const std::string& key,
                                           const base::Value* value) {
  FOR_EACH_OBSERVER(
      DBStore::Observer, observers_, OnDBValueChanged(key, value));
}

}  // namespace application
}  // namespace xwalk
