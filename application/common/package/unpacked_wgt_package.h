// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_PACKAGE_UNPACKED_WGT_PACKAGE_H_
#define XWALK_APPLICATION_COMMON_PACKAGE_UNPACKED_WGT_PACKAGE_H_

#include "base/files/file_path.h"
#include "xwalk/application/common/package/package.h"

namespace xwalk {
namespace application {

class UnpackedWGTPackage : public Package {
 public:
  explicit UnpackedWGTPackage(const base::FilePath& path);
  virtual ~UnpackedWGTPackage();
  bool ExtractToTemporaryDir(base::FilePath* result_path) override;
  bool ExtractTo(const base::FilePath& target_path) override;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_PACKAGE_UNPACKED_WGT_PACKAGE_H_
