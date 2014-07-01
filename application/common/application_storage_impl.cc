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
#include "xwalk/application/common/application_storage.h"
#include "xwalk/application/common/application_storage_constants.h"

namespace db_fields = xwalk::application_storage_constants;
namespace xwalk {
namespace application {

const base::FilePath::CharType ApplicationStorageImpl::kDBFileName[] =
    FILE_PATH_LITERAL("applications.db");

const char kPermissionSeparator = '^';

// Switching the JSON format DB(version 0) to SQLite backend version 1,
// should migrate all data from JSON DB to SQLite applications table.
static const int kVersionNumber = 1;

namespace {

const std::string StoredPermissionStr[] = {
    "ALLOW",
    "DENY",
    "PROMPT",
};

std::string ToString(StoredPermission permission) {
  if (permission == UNDEFINED_STORED_PERM)
    return std::string("");
  return StoredPermissionStr[permission];
}

StoredPermission ToPermission(const std::string& str) {
  unsigned int i;
  for (i = 0; i < UNDEFINED_STORED_PERM; ++i) {
    if (str == StoredPermissionStr[i])
      break;
  }
  return static_cast<StoredPermission>(i);
}

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

// Permissions are stored like "bluetooth^ALLOW;calendar^DENY;contacts^ALLOW"
std::string ToString(StoredPermissionMap permissions) {
  std::string str;
  StoredPermissionMap::iterator iter;
  for (iter = permissions.begin(); iter != permissions.end(); ++iter) {
    if (!str.empty())
      str += ";";
    str += (iter->first + kPermissionSeparator + ToString(iter->second));
  }
  return str;
}

StoredPermissionMap ToPermissionMap(const std::string& str) {
  StoredPermissionMap map;
  std::vector<std::string> vec;
  base::SplitString(str, ';', &vec);
  if (!vec.empty()) {
    for (std::vector<std::string>::iterator iter = vec.begin();
        iter != vec.end(); ++iter) {
      std::vector<std::string> perm_item;
      base::SplitString(*iter, kPermissionSeparator, &perm_item);
      if (perm_item.size() != 2) {
        LOG(ERROR) << "Permission format error! Corrupted database?";
        map.clear();
        break;
      }
      map[perm_item[0]] = ToPermission(perm_item[1]);
    }
  }
  return map;
}

bool InitPermissionsTable(sql::Connection* db) {
  sql::Transaction transaction(db);
  transaction.Begin();
  if (!db->DoesTableExist(db_fields::kPermissionTableName)) {
    if (!db->Execute(db_fields::kCreatePermissionTableOp))
     return false;
  }
  return transaction.Commit();
}

}  // namespace

ApplicationStorageImpl::ApplicationStorageImpl(const base::FilePath& path)
    : data_path_(path),
      db_initialized_(false) {
  // Ensure the parent directory for database file is created before reading
  // from it.
  if (!base::PathExists(path) && !base::CreateDirectory(path))
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

bool ApplicationStorageImpl::Init() {
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

  if (!InitPermissionsTable(sqlite_db.get())) {
    LOG(ERROR) << "Unable to open registered permissions table.";
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

  db_initialized_ = true;

  return db_initialized_;
}

scoped_refptr<ApplicationData> ApplicationStorageImpl::ExtractApplicationData(
    const sql::Statement& smt) {
  std::string id = smt.ColumnString(0);

  int error_code;
  std::string manifest_str = smt.ColumnString(1);
  JSONStringValueSerializer serializer(&manifest_str);
  std::string error_msg;
  scoped_ptr<base::DictionaryValue> manifest(
      static_cast<base::DictionaryValue*>(
          serializer.Deserialize(&error_code, &error_msg)));

  if (!manifest) {
    LOG(ERROR) << "An error occured when deserializing the manifest, "
                  "the error message is: "
               << error_msg;
    return NULL;
  }
  std::string path = smt.ColumnString(2);
  double install_time = smt.ColumnDouble(3);

  std::string error;
  scoped_refptr<ApplicationData> app_data =
      ApplicationData::Create(
          base::FilePath::FromUTF8Unsafe(path),
          Manifest::INTERNAL,
          *manifest,
          id,
          &error);
  if (!app_data) {
    LOG(ERROR) << "Load appliation error: " << error;
    return NULL;
  }

  app_data->install_time_ = base::Time::FromDoubleT(install_time);

  app_data->permission_map_ = ToPermissionMap(smt.ColumnString(4));

  return app_data;
}

scoped_refptr<ApplicationData> ApplicationStorageImpl::GetApplicationData(
    const std::string& app_id) {
  if (!db_initialized_) {
    LOG(ERROR) << "The database hasn't been initilized.";
    return NULL;
  }

  sql::Statement smt(sqlite_db_->GetUniqueStatement(
      db_fields::kGetRowFromAppTableOp));
  smt.BindString(0, app_id);
  if (!smt.is_valid())
    return NULL;

  if (!smt.Step())
    return NULL;

  return ExtractApplicationData(smt);
}

bool ApplicationStorageImpl::GetInstalledApplications(
    ApplicationData::ApplicationDataMap& applications) {  // NOLINT
  if (!db_initialized_) {
    LOG(ERROR) << "The database hasn't been initilized.";
    return false;
  }

  sql::Statement smt(sqlite_db_->GetUniqueStatement(
      db_fields::kGetAllRowsFromAppTableOp));
  if (!smt.is_valid())
    return false;

  while (smt.Step()) {
    scoped_refptr<ApplicationData> data = ExtractApplicationData(smt);
    if (!data) {
      LOG(ERROR) << "Failed to obtain ApplicationData from SQL query";
      return false;
    }
    applications.insert(
          std::pair<std::string, scoped_refptr<ApplicationData> >(
              data->ID(), data));
  }

  return true;
}

bool ApplicationStorageImpl::GetInstalledApplicationIDs(
  std::vector<std::string>& app_ids) {  // NOLINT
  if (!db_initialized_) {
    LOG(ERROR) << "The database hasn't been initilized.";
    return false;
  }

  sql::Statement smt(sqlite_db_->GetUniqueStatement(
      db_fields::kGetAllIDsFromAppTableOp));
  if (!smt.is_valid())
    return false;

  while (smt.Step()) {
    const std::string& id = smt.ColumnString(0);
    if (!ApplicationData::IsIDValid(id)) {
      LOG(ERROR) << "Failed to obtain Application ID from SQL query";
      return false;
    }
    app_ids.push_back(id);
  }

  return true;
}

bool ApplicationStorageImpl::AddApplication(const ApplicationData* application,
                                            const base::Time& install_time) {
  if (!db_initialized_) {
    LOG(ERROR) << "The database hasn't been initilized.";
    return false;
  }

  return (SetApplicationValue(
      application, install_time, db_fields::kSetApplicationWithBindOp) &&
          SetPermissions(application->ID(), application->permission_map_));
}

bool ApplicationStorageImpl::UpdateApplication(
    ApplicationData* application, const base::Time& install_time) {
  if (!db_initialized_) {
    LOG(ERROR) << "The database hasn't been initilized.";
    return false;
  }

  if (SetApplicationValue(
          application, install_time, db_fields::kUpdateApplicationWithBindOp) &&
      UpdatePermissions(application->ID(), application->permission_map_)) {
    return true;
  }

  return false;
}

bool ApplicationStorageImpl::RemoveApplication(const std::string& id) {
  if (!db_initialized_) {
    LOG(ERROR) << "The database hasn't been initilized.";
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

bool ApplicationStorageImpl::ContainsApplication(const std::string& app_id) {
  return GetApplicationData(app_id) != NULL;
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

bool ApplicationStorageImpl::SetPermissions(const std::string& id,
    const StoredPermissionMap& permissions) {
  if (!db_initialized_) {
    LOG(ERROR) << "Database is not initialized.";
    return false;
  }
  return SetPermissionsValue(id, permissions,
      db_fields::kInsertPermissionsWithBindOp);
}

bool ApplicationStorageImpl::UpdatePermissions(const std::string& id,
    const StoredPermissionMap& permissions) {
  if (!db_initialized_) {
    LOG(ERROR) << "Database is not initialized.";
    return false;
  }
  if (permissions.empty())
    return RevokePermissions(id);

  return SetPermissionsValue(id, permissions,
      db_fields::kUpdatePermissionsWithBindOp);
}

bool ApplicationStorageImpl::RevokePermissions(const std::string& id) {
  sql::Transaction transaction(sqlite_db_.get());
  if (!transaction.Begin())
    return false;

  sql::Statement statement(sqlite_db_->GetUniqueStatement(
      db_fields::kDeletePermissionsWithBindOp));
  statement.BindString(0, id);

  if (!statement.Run()) {
    LOG(ERROR) <<
        "An error occurred while removing permissions.";
    return false;
  }
  return transaction.Commit();
}

bool ApplicationStorageImpl::SetPermissionsValue(
    const std::string& id,
    const StoredPermissionMap& permissions,
    const std::string& operation) {
  sql::Transaction transaction(sqlite_db_.get());
  std::string permission_str = ToString(permissions);

  if (!transaction.Begin())
    return false;

  sql::Statement statement(sqlite_db_->GetUniqueStatement(
      operation.c_str()));
  statement.BindString(0, permission_str);
  statement.BindString(1, id);
  if (!statement.Run()) {
    LOG(ERROR) <<
        "An error occurred while inserting permissions.";
    return false;
  }
  return transaction.Commit();
}

}  // namespace application
}  // namespace xwalk
