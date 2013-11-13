// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/installer/package.h"

#include "base/file_util.h"
#include "base/logging.h"
#include "crypto/signature_verifier.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/browser/installer/wgt_package.h"
#include "xwalk/application/browser/installer/xpk_package.h"

namespace xwalk {
namespace application {

Package::Package() {
}

Package::Package(ScopedStdioHandle* file, bool is_valid)
  : file_(file),
    is_valid_(is_valid) {
}

Package::~Package() {
}

// static
scoped_ptr<Package> Package::Create(const base::FilePath& source_path) {
  // TODO(riju): create specific package type
  if (source_path.MatchesExtension(".xpk"))
      return XPKPackage::Create(source_path);
  else if (source_path.MatchesExtension(".wgt"))
     return WGTPackage::Create(source_path);

  LOG(ERROR) << "Invalid package type";
  return scoped_ptr<Package>();
}

}  // namespace application
}  // namespace xwalk
