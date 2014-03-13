// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_EXTENSION_APPLICATION_WIDGET_STORAGE_H_
#define XWALK_APPLICATION_EXTENSION_APPLICATION_WIDGET_STORAGE_H_

#include <map>
#include <string>

#include "base/values.h"
#include "base/files/file_path.h"
#include "sql/connection.h"
#include "xwalk/application/browser/application.h"

namespace xwalk {
namespace application {

class AppWidgetStorage {
 public:
  AppWidgetStorage(Application* application,
                   const base::FilePath& data_dir);
  ~AppWidgetStorage();

  // Adds or replaces entry (if not readonly);
  // returns true on success.
  bool AddEntry(const std::string& key,
               const std::string& value,
               bool read_only);
  bool RemoveEntry(const std::string& key);
  bool Clear();
  bool GetAllEntries(base::DictionaryValue* result);
  bool EntryExists(const std::string& key) const;

 private:
  bool Init();
  bool IsReadOnly(const std::string& key);
  bool InitStorageTable();
  bool SaveConfigInfoInDB();
  bool SaveConfigInfoItem(base::DictionaryValue* dict);

  Application* application_;
  scoped_ptr<sql::Connection> sqlite_db_;
  base::FilePath data_path_;
  bool db_initialized_;
};

}  // namespace application
}  // namespace xwalk
#endif  // XWALK_APPLICATION_EXTENSION_APPLICATION_WIDGET_STORAGE_H_
