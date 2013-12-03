// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_APPLICATION_STORAGE_IMPL_H_
#define XWALK_APPLICATION_COMMON_APPLICATION_STORAGE_IMPL_H_

#include <set>
#include <string>
#include <utility>

#include "base/files/file_path.h"
#include "sql/connection.h"
#include "sql/meta_table.h"
#include "xwalk/application/common/application_data.h"

namespace xwalk {
namespace application {

// The Sqlite backend implementation of ApplicationStorage.
class ApplicationStorageImpl {
 public:
  static const base::FilePath::CharType kDBFileName[];
  explicit ApplicationStorageImpl(const base::FilePath& path);
  ~ApplicationStorageImpl();

  bool AddApplication(const ApplicationData* application,
                      const base::Time& install_time);
  bool RemoveApplication(const std::string& key);
  bool UpdateApplication(ApplicationData* application,
                         const base::Time& install_time);
  bool Init(ApplicationData::ApplicationDataMap& applications);
  bool GetInstalledApplications(
      ApplicationData::ApplicationDataMap& applications);

 private:
  bool UpgradeToVersion1(const base::FilePath& v0_file);
  bool SetApplicationValue(const ApplicationData* application,
                           const base::Time& install_time,
                           const std::string& operation);

  bool SetEventsValue(const std::string& id,
                      const std::set<std::string>& events,
                      const std::string& operation);
  bool SetEvents(const std::string& id,
                 const std::set<std::string>& events);
  bool UpdateEvents(const std::string& id,
                    const std::set<std::string>& events);
  bool DeleteEvents(const std::string& id);

  scoped_ptr<sql::Connection> sqlite_db_;
  sql::MetaTable meta_table_;
  base::FilePath data_path_;
  bool db_initialized_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_APPLICATION_STORAGE_IMPL_H_
