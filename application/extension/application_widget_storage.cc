// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/extension/application_widget_storage.h"

#include <string>

#include "base/file_util.h"
#include "base/strings/utf_string_conversions.h"
#include "sql/statement.h"
#include "sql/transaction.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/widget_handler.h"

namespace {

namespace widget_keys = xwalk::application_widget_keys;

const char kPreferences[] = "preferences";
const char kPreferencesName[] = "name";
const char kPreferencesValue[] = "value";
const char kPreferencesReadonly[] = "readonly";

const base::FilePath::CharType kWidgetStorageExtension[] =
    FILE_PATH_LITERAL(".widgetStorage");

const char kStorageTableName[] = "widget_storage";

const char kCreateStorageTableOp[] =
    "CREATE TABLE widget_storage ("
    "key TEXT NOT NULL UNIQUE PRIMARY KEY,"
    "value TEXT NOT NULL,"
    "read_only INTEGER )";

const char kClearStorageTableWithBindOp[] =
    "DELETE FROM widget_storage WHERE read_only = ? ";

const char kInsertItemWithBindOp[] =
    "INSERT INTO widget_storage (value, read_only, key) "
    "VALUES(?,?,?)";

const char kUpdateItemWithBindOp[] =
    "UPDATE widget_storage SET value = ? , read_only = ? "
    "WHERE key = ?";

const char kRemoveItemWithBindOp[] =
    "DELETE FROM widget_storage WHERE key = ?";

const char kSelectTableLength[] =
    "SELECT count(*) FROM widget_storage ";

const char kSelectCountWithBindOp[] =
    "SELECT count(*) FROM widget_storage "
    "WHERE key = ?";

const char kSelectAllItem[] =
    "SELECT key, value FROM widget_storage ";

const char kSelectReadOnlyWithBindOp[] =
    "SELECT read_only FROM widget_storage "
    "WHERE key = ?";
}  // namespace

namespace xwalk {
namespace application {

AppWidgetStorage::AppWidgetStorage(Application* application,
                                   const base::FilePath& data_dir)
    : application_(application),
      db_initialized_(false) {
  sqlite_db_.reset(new sql::Connection);

  base::FilePath name;
#if defined(OS_WIN)
  name = base::FilePath(base::UTF8ToWide(application_->id()));
#else
  name = base::FilePath(application_->id());
#endif
  base::FilePath::StringType storage_name =
      name.value() + kWidgetStorageExtension;
  data_path_ = data_dir.Append(storage_name);

  if (!base::PathExists(data_dir) && !base::CreateDirectory(data_dir)) {
    LOG(ERROR) << "Could not create widget storage path.";
    return;
  }

  if (!Init()) {
    LOG(ERROR) << "Initialize widget storage failed.";
    return;
  }
}

AppWidgetStorage::~AppWidgetStorage() {
}

bool AppWidgetStorage::Init() {
  if (!sqlite_db_->Open(data_path_)) {
    LOG(ERROR) << "Unable to open widget storage DB.";
    return false;
  }
  sqlite_db_->Preload();

  if (!InitStorageTable()) {
     LOG(ERROR) << "Unable to init widget storage table.";
     return false;
  }
  return db_initialized_;
}

bool AppWidgetStorage::SaveConfigInfoItem(base::DictionaryValue* dict) {
  DCHECK(dict);
  std::string key;
  std::string value;
  bool read_only = false;
  dict->GetString(kPreferencesName, &key);
  dict->GetString(kPreferencesValue, &value);
  dict->GetBoolean(kPreferencesReadonly, &read_only);
  return AddEntry(key, value, read_only);
}

bool AppWidgetStorage::SaveConfigInfoInDB() {
  WidgetInfo* info =
      static_cast<WidgetInfo*>(
      application_->data()->GetManifestData(widget_keys::kWidgetKey));
  base::DictionaryValue* widget_info = info->GetWidgetInfo();
  if (!widget_info) {
    LOG(ERROR) << "Fail to get parsed widget information.";
    return false;
  }

  base::Value* pref_value;
  widget_info->Get(kPreferences, &pref_value);

  if (pref_value && pref_value->IsType(base::Value::TYPE_DICTIONARY)) {
    base::DictionaryValue* dict;
    pref_value->GetAsDictionary(&dict);
    return SaveConfigInfoItem(dict);
  } else if (pref_value && pref_value->IsType(base::Value::TYPE_LIST)) {
    base::ListValue* list;
    pref_value->GetAsList(&list);

    for (base::ListValue::iterator it = list->begin();
         it != list->end(); ++it) {
      base::DictionaryValue* dict;
      (*it)->GetAsDictionary(&dict);
      if (!SaveConfigInfoItem(dict))
        return false;
    }
  } else {
    LOG(INFO) << "No widget preferences or preference type is not supported.";
  }

  return true;
}

bool AppWidgetStorage::InitStorageTable() {
  if (sqlite_db_->DoesTableExist(kStorageTableName)) {
    db_initialized_ = (sqlite_db_ && sqlite_db_->is_open());
    return true;
  }

  sql::Transaction transaction(sqlite_db_.get());
  transaction.Begin();
  if (!sqlite_db_->Execute(kCreateStorageTableOp))
    return false;
  if (!transaction.Commit())
    return false;

  db_initialized_ = (sqlite_db_ && sqlite_db_->is_open());
  SaveConfigInfoInDB();

  return true;
}

bool AppWidgetStorage::EntryExists(const std::string& key) const {
  sql::Transaction transaction(sqlite_db_.get());
  if (!transaction.Begin())
    return false;

  sql::Statement stmt(sqlite_db_->GetUniqueStatement(
      kSelectCountWithBindOp));
  stmt.BindString(0, key);
  if (!stmt.Step()) {
    LOG(ERROR) << "An error occured when selecting count from DB.";
    return false;
  }

  if (!transaction.Commit())
    return false;

  int exist = stmt.ColumnInt(0);
  return exist > 0;
}

bool AppWidgetStorage::IsReadOnly(const std::string& key) {
  sql::Transaction transaction(sqlite_db_.get());
  if (!transaction.Begin())
    return true;

  sql::Statement stmt(sqlite_db_->GetUniqueStatement(
      kSelectReadOnlyWithBindOp));
  stmt.BindString(0, key);
  if (!stmt.Step()) {
    LOG(ERROR) << "An error occured when selecting count from DB.";
    return true;
  }

  if (!transaction.Commit())
    return true;

  return stmt.ColumnBool(0);
}

bool AppWidgetStorage::AddEntry(const std::string& key,
                               const std::string& value,
                               bool read_only) {
  if (!db_initialized_ && !Init())
    return false;

  std::string operation;
  if (!EntryExists(key)) {
    operation = kInsertItemWithBindOp;
  } else if (!IsReadOnly(key)) {
    operation = kUpdateItemWithBindOp;
  } else {
    LOG(ERROR) << "Could not set read only item " << key;
    return false;
  }

  sql::Transaction transaction(sqlite_db_.get());
  if (!transaction.Begin())
    return false;

  sql::Statement stmt(sqlite_db_->GetUniqueStatement(
    operation.c_str()));
  stmt.BindString(0, value);
  stmt.BindBool(1, read_only);
  stmt.BindString(2, key);
  if (!stmt.Run()) {
    LOG(ERROR) << "An error occured when set item into DB.";
    return false;
  }

  return transaction.Commit();
}

bool AppWidgetStorage::RemoveEntry(const std::string& key) {
  if (!db_initialized_ && !Init())
    return false;

  if (IsReadOnly(key)) {
    LOG(ERROR) << "Could not remove read only item " << key;
    return false;
  }

  sql::Transaction transaction(sqlite_db_.get());
  if (!transaction.Begin())
    return false;

  sql::Statement stmt(sqlite_db_->GetUniqueStatement(
      kRemoveItemWithBindOp));
  stmt.BindString(0, key);

  if (!stmt.Run()) {
    LOG(ERROR) << "An error occured when removing item into DB.";
    return false;
  }

  return transaction.Commit();
}

bool AppWidgetStorage::Clear() {
  if (!db_initialized_ && !Init())
    return false;

  sql::Transaction transaction(sqlite_db_.get());
  transaction.Begin();

  sql::Statement stmt(sqlite_db_->GetUniqueStatement(
      kClearStorageTableWithBindOp));
  stmt.BindBool(0, false);

  if (!stmt.Run()) {
    LOG(ERROR) << "An error occured when removing item into DB.";
    return false;
  }

  return transaction.Commit();
}

bool AppWidgetStorage::GetAllEntries(base::DictionaryValue* result) {
  std::string key;
  std::string value;
  DCHECK(result);

  if (!db_initialized_ && !Init())
    return false;

  sql::Statement stmt(sqlite_db_->GetUniqueStatement(kSelectAllItem));
  while (stmt.Step()) {
    key = stmt.ColumnString(0);
    value = stmt.ColumnString(1);
    result->SetString(key, value);
  }

  return true;
}

}  // namespace application
}  // namespace xwalk
