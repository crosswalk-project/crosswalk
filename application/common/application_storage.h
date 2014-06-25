// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_APPLICATION_STORAGE_H_
#define XWALK_APPLICATION_COMMON_APPLICATION_STORAGE_H_

#include <map>
#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "xwalk/application/common/application_data.h"

namespace xwalk {
namespace application {

class ApplicationStorage {
 public:
  explicit ApplicationStorage(const base::FilePath& path);
  ~ApplicationStorage();

  bool AddApplication(scoped_refptr<ApplicationData> app_data);

  bool RemoveApplication(const std::string& app_id);

  bool UpdateApplication(scoped_refptr<ApplicationData> app_data);

  bool Contains(const std::string& app_id) const;

  scoped_refptr<ApplicationData> GetApplicationData(
      const std::string& app_id) const;

  bool GetInstalledApplications(
      ApplicationData::ApplicationDataMap& apps) const;  // NOLINT

 private:
  scoped_ptr<class ApplicationStorageImpl> impl_;
  DISALLOW_COPY_AND_ASSIGN(ApplicationStorage);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_APPLICATION_STORAGE_H_
