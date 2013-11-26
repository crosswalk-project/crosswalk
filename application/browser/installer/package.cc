// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/installer/package.h"

#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "third_party/zlib/google/zip.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/browser/installer/wgt_package.h"
#include "xwalk/application/browser/installer/xpk_package.h"

namespace xwalk {
namespace application {

Package::Package(const base::FilePath& source_path)
  : source_path_(source_path) {
}

Package::~Package() {
}

// static
scoped_ptr<Package> Package::Create(const base::FilePath& source_path) {
  if (source_path.MatchesExtension(FILE_PATH_LITERAL(".xpk"))) {
      scoped_ptr<Package> package(new XPKPackage(source_path));
      if (!package->IsValid())
        LOG(ERROR) << "Package not valid";
      return package.Pass();
  } else if (source_path.MatchesExtension(FILE_PATH_LITERAL(".wgt"))) {
     scoped_ptr<Package> package(new WGTPackage(source_path));
     return package.Pass();
  }

  LOG(ERROR) << "Invalid package type. Only .xpk/.wgt supported now";
  return scoped_ptr<Package>();
}

bool Package::Extract(base::FilePath* target_path) {
  if (!IsValid()) {
    LOG(ERROR) << "XPK/WGT file is not valid.";
    return false;
  }

  if (!CreateTempDirectory()) {
    LOG(ERROR) << "Can't create a temporary"
                  "directory for extracting the package content.";
    return false;
  }

  if (!zip::Unzip(source_path_, temp_dir_.path())) {
    LOG(ERROR) << "An error occurred during package extraction";
    return false;
  }

  *target_path = temp_dir_.path();
  return true;
}

// Create a temporary directory to decompress the zipped package file.
// As the package information might already exists under data_path,
// it's safer to extract the XPK/WGT file into a temporary directory first.
bool Package::CreateTempDirectory() {
  base::FilePath tmp;
  PathService::Get(base::DIR_TEMP, &tmp);
  if (tmp.empty())
    return false;
  if (!temp_dir_.CreateUniqueTempDirUnderPath(tmp))
    return false;
  return true;
}

}  // namespace application
}  // namespace xwalk
