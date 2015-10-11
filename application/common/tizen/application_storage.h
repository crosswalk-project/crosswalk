// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_TIZEN_APPLICATION_STORAGE_H_
#define XWALK_APPLICATION_COMMON_TIZEN_APPLICATION_STORAGE_H_

#include <string>
#include <vector>

#include "xwalk/application/common/application_data.h"

namespace xwalk {
namespace application {

class ApplicationStorage {
 public:
  explicit ApplicationStorage(const base::FilePath& path);
  ~ApplicationStorage();

  bool Contains(const std::string& app_id) const;

  scoped_refptr<ApplicationData> GetApplicationData(
      const std::string& app_id) const;

  bool GetInstalledApplicationIDs(
      std::vector<std::string>& app_ids) const;  // NOLINT

 private:
  DISALLOW_COPY_AND_ASSIGN(ApplicationStorage);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_TIZEN_APPLICATION_STORAGE_H_
