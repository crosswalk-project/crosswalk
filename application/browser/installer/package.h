// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_INSTALLER_PACKAGE_H_
#define XWALK_APPLICATION_BROWSER_INSTALLER_PACKAGE_H_

#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/scoped_handle.h"
#include "base/memory/scoped_ptr.h"

namespace xwalk {
namespace application {

// Base class for all types of packages (right now .wgt and .xpk)
// The actual zip file , id and is_valid_ are common to all the package classes
// specifics like signature checking for XPK are taken care of in
//  XPKPackage::Validate()
class Package {
 public:
  virtual ~Package();
  bool IsValid() const { return is_valid_; }
  const std::string& Id() const { return id_; }
  static scoped_ptr<Package> Create(const base::FilePath& path);
 protected:
  Package();
  Package(ScopedStdioHandle* file, bool is_valid);
  scoped_ptr<ScopedStdioHandle> file_;
  bool is_valid_;
  std::string id_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_INSTALLER_PACKAGE_H_
