// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_INSTALLER_PACKAGE_H_
#define XWALK_APPLICATION_BROWSER_INSTALLER_PACKAGE_H_

#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/scoped_handle.h"
#include "base/memory/scoped_ptr.h"

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
  // Factory method for creating a package
  static scoped_ptr<Package> Create(const base::FilePath& path);
  // The function will unzip the XPK/WGT file and return the target path where
  // to decompress by the parameter |target_path|.
  bool Extract(base::FilePath* target_path);
 protected:
  explicit Package(const base::FilePath& source_path);
  scoped_ptr<ScopedStdioHandle> file_;
  bool is_valid_;
  std::string id_;
  // Unzipping of the zipped file happens in a temporary directory
  bool CreateTempDirectory();
  base::FilePath source_path_;
  // Temporary directory for unpacking.
  base::ScopedTempDir temp_dir_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_INSTALLER_PACKAGE_H_
