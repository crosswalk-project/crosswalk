// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_INSTALLER_TIZEN_SERVICE_PACKAGE_INSTALLER_H_
#define XWALK_APPLICATION_BROWSER_INSTALLER_TIZEN_SERVICE_PACKAGE_INSTALLER_H_

#include <string>
#include "base/files/file_path.h"

namespace xwalk {
namespace application {

class ApplicationData;

bool InstallApplicationForTizen(ApplicationData* application,
                                const base::FilePath& data_dir);

bool UninstallApplicationForTizen(ApplicationData* application,
                                  const base::FilePath& data_dir);

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_INSTALLER_TIZEN_SERVICE_PACKAGE_INSTALLER_H_
