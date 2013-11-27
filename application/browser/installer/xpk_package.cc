// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/installer/xpk_package.h"

#include "base/file_util.h"
#include "crypto/signature_verifier.h"
#include "xwalk/application/common/id_util.h"

namespace xwalk {
namespace application {

const uint8 kSignatureAlgorithm[15] = {
  0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
  0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00
};

const char XPKPackage::kXPKPackageHeaderMagic[] = "CrWk";

XPKPackage::~XPKPackage() {
}

XPKPackage::XPKPackage(const base::FilePath& path)
  : Package(path) {
  if (!base::PathExists(path))
    return;
  scoped_ptr<ScopedStdioHandle> file(
      new ScopedStdioHandle(file_util::OpenFile(path, "rb")));
  file_ = file.Pass();
  size_t len = fread(&header_, 1, sizeof(header_), file_->get());
  is_valid_ = false;
  if (len < sizeof(header_))
    return;
  if (!strncmp(XPKPackage::kXPKPackageHeaderMagic,
               header_.magic,
               sizeof(header_.magic)) &&
      header_.key_size > 0 &&
      header_.key_size <= XPKPackage::kMaxPublicKeySize &&
      header_.signature_size > 0 &&
      header_.signature_size <= XPKPackage::kMaxSignatureKeySize) {
      is_valid_ = true;
      zip_addr_ = sizeof(header_) + header_.key_size + header_.signature_size;
        fseek(file_->get(), sizeof(header_), SEEK_SET);
        key_.resize(header_.key_size);
        size_t len = fread(
            &key_.front(), sizeof(uint8), header_.key_size, file_->get());
        if (len < header_.key_size)
          is_valid_ = false;

        signature_.resize(header_.signature_size);
        len = fread(&signature_.front(),
                    sizeof(uint8),
                    header_.signature_size,
                    file_->get());
        if (len < header_.signature_size)
          is_valid_ = false;

        if (!VerifySignature())
          is_valid_ = false;

        std::string public_key =
            std::string(reinterpret_cast<char*>(&key_.front()), key_.size());
        id_ = GenerateId(public_key);
  }
  return;
}

bool XPKPackage::VerifySignature() {
// Set the file read position to the beginning of compressed resource file,
// which is behind the magic header, public key and signature key.
  fseek(file_->get(), zip_addr_, SEEK_SET);
  crypto::SignatureVerifier verifier;
  if (!verifier.VerifyInit(kSignatureAlgorithm,
                           sizeof(kSignatureAlgorithm),
                           &signature_.front(),
                           signature_.size(),
                           &key_.front(),
                           key_.size()))
    return false;
  unsigned char buf[1 << 12];
  size_t len = 0;
  while ((len = fread(buf, 1, sizeof(buf), file_->get())) > 0)
    verifier.VerifyUpdate(buf, len);
  if (!verifier.VerifyFinal())
    return false;

  return true;
}

}  // namespace application
}  // namespace xwalk
