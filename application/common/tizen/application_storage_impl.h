// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_TIZEN_APPLICATION_STORAGE_IMPL_H_
#define XWALK_APPLICATION_COMMON_TIZEN_APPLICATION_STORAGE_IMPL_H_

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "base/files/file_path.h"
#include "sql/connection.h"
#include "sql/meta_table.h"
#include "xwalk/application/common/application_data.h"

namespace xwalk {
namespace application {

// The Sqlite backend implementation of ApplicationStorage.
class ApplicationStorageImpl {
 public:
  explicit ApplicationStorageImpl(const base::FilePath& path);
  ~ApplicationStorageImpl();

  bool AddApplication(const ApplicationData* application,
                      const base::Time& install_time);
  bool RemoveApplication(const std::string& key);
  bool ContainsApplication(const std::string& key);
  bool UpdateApplication(ApplicationData* application,
                         const base::Time& install_time);
  bool Init();

  scoped_refptr<ApplicationData> GetApplicationData(const std::string& id);

  bool GetInstalledApplicationIDs(
      std::vector<std::string>& app_ids);  // NOLINT
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_TIZEN_APPLICATION_STORAGE_IMPL_H_
