// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/installer/wgt_package.h"

#include "base/file_util.h"
#include "xwalk/application/common/id_util.h"

namespace xwalk {
namespace application {

WGTPackage::~WGTPackage() {
}

WGTPackage::WGTPackage(PackageType pkg_type, const base::FilePath& path)
  : Package(pkg_type, path) {
  if (!base::PathExists(path))
    return;
  scoped_ptr<ScopedStdioHandle> file(
        new ScopedStdioHandle(file_util::OpenFile(path, "rb")));
  file_ = file.Pass();
  // TODO(riju): check for validation of wgt file
  is_valid_ = true;
  // id_ = "nrT4AQuzWO";
}

}  // namespace application
}  // namespace xwalk



