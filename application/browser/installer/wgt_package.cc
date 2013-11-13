// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/installer/wgt_package.h"

#include "base/file_util.h"
#include "xwalk/application/common/id_util.h"

namespace xwalk {
namespace application {

WGTPackage::WGTPackage() {
}

WGTPackage::~WGTPackage() {
}

// static
scoped_ptr<Package>WGTPackage::Create(const base::FilePath& path) {
  if (!base::PathExists(path))
    scoped_ptr<Package>();
  scoped_ptr<ScopedStdioHandle> file(
        new ScopedStdioHandle(file_util::OpenFile(path, "rb")));
  // TODO(riju) : see if any pre processing is required before parsing)
  scoped_ptr<Package> package(new WGTPackage(file.release()));
  if (package->IsValid())
    return package.Pass();

  return scoped_ptr<Package>();
}

WGTPackage::WGTPackage(ScopedStdioHandle* file)
  : Package(file, true) {
}

}  // namespace application
}  // namespace xwalk



