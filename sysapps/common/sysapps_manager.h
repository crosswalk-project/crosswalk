// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_COMMON_SYSAPPS_MANAGER_H_
#define XWALK_SYSAPPS_COMMON_SYSAPPS_MANAGER_H_

#include "xwalk/extensions/common/xwalk_extension_vector.h"

namespace xwalk {
namespace sysapps {

using extensions::XWalkExtensionVector;

class CPUInfoProvider;

// This class manages the registration of the SysApps APIs. It will append
// to the list of extensions the SysApps APIs, taking the features flags into
// account. We also use this class to manage the lifecycle of data providers
// shared across multiple extensions.
class SysAppsManager {
 public:
  SysAppsManager();
  ~SysAppsManager();

  void CreateExtensionsForUIThread(XWalkExtensionVector* extensions);
  void CreateExtensionsForExtensionThread(XWalkExtensionVector* extensions);

  static CPUInfoProvider* GetCPUInfoProvider();
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_COMMON_SYSAPPS_MANAGER_H_
