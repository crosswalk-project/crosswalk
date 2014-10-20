// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_PACKAGE_WGT_PACKAGE_H_
#define XWALK_APPLICATION_COMMON_PACKAGE_WGT_PACKAGE_H_

#include "base/files/file_path.h"
#include "xwalk/application/common/package/package.h"

namespace xwalk {
namespace application {

class WGTPackage : public Package {
 public:
  explicit WGTPackage(const base::FilePath& path);
  virtual ~WGTPackage();
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_PACKAGE_WGT_PACKAGE_H_
