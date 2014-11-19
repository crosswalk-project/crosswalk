// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_PACKAGE_PACKAGE_H_
#define XWALK_APPLICATION_COMMON_PACKAGE_PACKAGE_H_

#include <string>

#include "base/files/file_path.h"
#include "base/files/scoped_file.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/application/common/manifest.h"

namespace xwalk {
namespace application {

// Base class for all types of packages (right now .wgt and .xpk)
// The actual zip file, id, is_valid_, source_path_ are common in all packages
// specifics like signature checking for XPK are taken care of in
//  XPKPackage::Validate()
class Package {
 public:
  virtual ~Package();
  bool IsValid() const { return is_valid_; }
  const std::string& Id() const { return id_; }
  const std::string& name() const { return name_; }
  // Returns the type of the manifest which the package contains.
  Manifest::Type manifest_type() const { return manifest_type_; }
  // Factory method for creating a package
  static scoped_ptr<Package> Create(const base::FilePath& path);
  // The function will unzip the XPK/WGT file and return the target path where
  // to decompress by the parameter |target_path|.
  virtual bool ExtractToTemporaryDir(base::FilePath* result_path);
  // The function will unzip the XPK/WGT file to the given folder.
  virtual bool ExtractTo(const base::FilePath& target_path);

 protected:
  Package(const base::FilePath& source_path, Manifest::Type manifest_type);
  // Unzipping of the zipped file happens in a temporary directory
  bool CreateTempDirectory();
  scoped_ptr<base::ScopedFILE> file_;

  bool is_valid_;
  base::FilePath source_path_;
  std::string id_;
  std::string name_;
  // Temporary directory for unpacking.
  base::ScopedTempDir temp_dir_;
  // Represent if the package has been extracted.
  bool is_extracted_;
  Manifest::Type manifest_type_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_PACKAGE_PACKAGE_H_
