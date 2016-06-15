// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/package/package.h"

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "third_party/zlib/google/zip.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/common/package/wgt_package.h"
#include "xwalk/application/common/package/xpk_package.h"

namespace xwalk {
namespace application {

Package::Package(const base::FilePath& source_path,
    Manifest::Type manifest_type)
    : is_valid_(false),
      source_path_(source_path),
      name_(source_path_.BaseName().AsUTF8Unsafe()),
      is_extracted_(false),
      manifest_type_(manifest_type) {
}

Package::~Package() {
}

// static
std::unique_ptr<Package> Package::Create(const base::FilePath& source_path) {
  if (source_path.MatchesExtension(FILE_PATH_LITERAL(".xpk"))) {
    std::unique_ptr<Package> package(new XPKPackage(source_path));
    return package;
  }
  if (source_path.MatchesExtension(FILE_PATH_LITERAL(".wgt"))) {
    std::unique_ptr<Package> package(new WGTPackage(source_path));
    return package;
  }

  LOG(ERROR) << "Invalid package type. Only .xpk/.wgt supported now";
  return std::unique_ptr<Package>();
}

bool Package::ExtractToTemporaryDir(base::FilePath* target_path) {
  if (is_extracted_) {
    *target_path = temp_dir_.path();
    return true;
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

  is_extracted_ = true;

  *target_path = temp_dir_.path();
  return true;
}

bool Package::ExtractTo(const base::FilePath& target_path) {
  if (!DirectoryExists(target_path)) {
    LOG(ERROR) << "The directory " << target_path.MaybeAsASCII()
               << "does not exist";
    return false;
  }
  if (!IsDirectoryEmpty(target_path)) {
    LOG(ERROR) << "The directory " << target_path.MaybeAsASCII()
               << "is not empty.";
    return false;
  }
  if (!zip::Unzip(source_path_, target_path)) {
    LOG(ERROR) << "An error occurred during package extraction";
    return false;
  }

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
