// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_INSTALLER_XPK_PACKAGE_H_
#define XWALK_APPLICATION_BROWSER_INSTALLER_XPK_PACKAGE_H_

#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/scoped_handle.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/application/browser/installer/package.h"

namespace xwalk {
namespace application {

class XPKPackage : public Package {
 public:
  static const char kXPKPackageHeaderMagic[];
  static const size_t kXPKPackageHeaderMagicSize = 4;
  static const uint32 kMaxPublicKeySize = 1 << 16;
  static const uint32 kMaxSignatureKeySize = 1 << 16;

  struct Header {
    char magic[kXPKPackageHeaderMagicSize];
    uint32 key_size;
    uint32 signature_size;
  };
  virtual ~XPKPackage();
  explicit XPKPackage(const base::FilePath& path);

 private:
  // verify the signature in the xpk package
  virtual bool VerifySignature();

  Header header_;
  std::vector<uint8> signature_;
  std::vector<uint8> key_;
  // It's the beginning address of the zip file
  int zip_addr_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_INSTALLER_XPK_PACKAGE_H_
