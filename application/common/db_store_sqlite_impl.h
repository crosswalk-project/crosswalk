// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_DB_STORE_SQLITE_IMPL_H_
#define XWALK_APPLICATION_COMMON_DB_STORE_SQLITE_IMPL_H_

#include <string>

#include "base/files/file_path.h"
#include "sql/connection.h"
#include "sql/meta_table.h"
#include "xwalk/application/common/db_store.h"

namespace xwalk {
namespace application {

// The Sqlite backend implementation of DBStore.
class DBStoreSqliteImpl: public DBStore {
 public:
  static const base::FilePath::CharType kDBFileName[];
  explicit DBStoreSqliteImpl(const base::FilePath& path);
  virtual ~DBStoreSqliteImpl();

  // Implement the DBStore inferface.
  virtual bool Insert(const Application* application,
                      const base::Time install_time) OVERRIDE;
  virtual bool Remove(const std::string& key) OVERRIDE;
  virtual bool InitDB() OVERRIDE;
  virtual void SetValue(const std::string& key, base::Value* value) OVERRIDE;

 private:
  enum Action {
    ACTION_UNKNOWN = 0,
    ACTION_INSERT,
    ACTION_UPDATE,
    ACTION_DELETE
  };
  bool UpdateDBCache();
  bool Commit(const std::string& id,
              const std::string& column,
              base::Value* value,
              Action action);
  void ReportValueChanged(const std::string& key,
                          const base::Value* value);
  bool UpgradeToVersion1(const base::FilePath& v0_file);
  bool SetApplication(const std::string& id, base::Value* value);
  bool UpdateApplication(const std::string& id, base::Value* value);
  bool DeleteApplication(const std::string& id);
  bool SetManifestValue(const std::string& id, base::Value* value);
  bool SetInstallTimeValue(const std::string& id, base::Value* value);
  bool SetApplicationPathValue(const std::string& id, base::Value* value);

  bool SetEventsValue(const std::string& id,
                      base::Value* events,
                      const std::string& operation);
  bool DeleteEventsValue(const std::string& id);

  scoped_ptr<sql::Connection> sqlite_db_;
  sql::MetaTable meta_table_;
  bool db_initialized_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_DB_STORE_SQLITE_IMPL_H_
