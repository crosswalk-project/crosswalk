// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_INSTALLER_XPK_EXTRACTOR_H_
#define XWALK_APPLICATION_BROWSER_INSTALLER_XPK_EXTRACTOR_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "base/files/scoped_temp_dir.h"
#include "xwalk/application/browser/installer/xpk_package.h"

namespace xwalk {
namespace application {

class XPKExtractor
    : public base::RefCountedThreadSafe<XPKExtractor> {
 public:
  static scoped_refptr<XPKExtractor> Create(const base::FilePath& source_path);
  // The function will unzip the XPK file and return the target path where
  // to decompress by the parameter |target_path|.
  bool Extract(base::FilePath* target_path);
  std::string GetPackageID() const;

 private:
  friend class base::RefCountedThreadSafe<XPKExtractor>;
  explicit XPKExtractor(const base::FilePath& source_path);
  bool CreateTempDirectory();

  base::FilePath source_path_;
  // Temporary directory for unpacking.
  base::ScopedTempDir temp_dir_;
  scoped_ptr<XPKPackage> xpk_package_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_INSTALLER_XPK_EXTRACTOR_H_
