// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_TIZEN_PACKAGE_QUERY_H_
#define XWALK_APPLICATION_COMMON_TIZEN_PACKAGE_QUERY_H_

#include <string>

#include "base/files/file_util.h"
#include "base/time/time.h"

namespace xwalk {
namespace application {

base::FilePath GetApplicationPath(const std::string& app_id);
base::FilePath GetPackagePath(const std::string& pkg_id);

std::string GetPackageType(const std::string& pkg_id);

base::Time GetApplicationInstallationTime(const std::string& app_id);

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_TIZEN_PACKAGE_QUERY_H_
