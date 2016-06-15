// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/package/xpk_package.h"

#include <string>

#include "base/files/file_util.h"
#include "base/files/scoped_file.h"
#include "base/numerics/safe_conversions.h"
#include "crypto/signature_verifier.h"
#include "xwalk/application/common/id_util.h"

namespace xwalk {
namespace application {

const uint8_t kSignatureAlgorithm[15] = {
  0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
  0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00
};

const char XPKPackage::kXPKPackageHeaderMagic[] = "CrWk";

XPKPackage::~XPKPackage() {
}

XPKPackage::XPKPackage(const base::FilePath& path)
    : Package(path, Manifest::TYPE_MANIFEST),
      header_(),
      zip_addr_(0) {
  if (!base::PathExists(path))
    return;
  std::unique_ptr<base::ScopedFILE> file(
      new base::ScopedFILE(base::OpenFile(path, "rb")));
  file_ = std::move(file);
  size_t len = fread(&header_, 1, sizeof(header_), file_->get());
  is_valid_ = false;
  if (len < sizeof(header_))
    return;
  if (!strncmp(XPKPackage::kXPKPackageHeaderMagic, header_.magic,
               sizeof(header_.magic)) &&
      header_.key_size > 0 &&
      header_.key_size <= XPKPackage::kMaxPublicKeySize &&
      header_.signature_size > 0 &&
      header_.signature_size <= XPKPackage::kMaxSignatureKeySize) {
    is_valid_ = true;
    zip_addr_ = sizeof(header_) + header_.key_size + header_.signature_size;
    if (fseek(file_->get(), sizeof(header_), SEEK_SET)) {
      is_valid_ = false;
      return;
    }
    key_.resize(header_.key_size);
    size_t len = fread(&key_.front(), sizeof(uint8_t), header_.key_size,
        file_->get());
    if (len < header_.key_size)
      is_valid_ = false;

    signature_.resize(header_.signature_size);
    len = fread(&signature_.front(), sizeof(uint8_t), header_.signature_size,
        file_->get());
    if (len < header_.signature_size)
      is_valid_ = false;

    if (!VerifySignature())
      is_valid_ = false;

    std::string public_key =
        std::string(reinterpret_cast<char*>(&key_.front()), key_.size());
    id_ = GenerateId(public_key);
  }
}

bool XPKPackage::VerifySignature() {
// Set the file read position to the beginning of compressed resource file,
// which is behind the magic header, public key and signature key.
  if (fseek(file_->get(), zip_addr_, SEEK_SET))
    return false;
  crypto::SignatureVerifier verifier;
  if (!verifier.VerifyInit(crypto::SignatureVerifier::RSA_PKCS1_SHA1,
                           &signature_.front(),
                           base::checked_cast<int>(signature_.size()),
                           &key_.front(),
                           base::checked_cast<int>(key_.size())))
    return false;
  unsigned char buf[1 << 12];
  size_t len = 0;
  while ((len = fread(buf, 1, sizeof(buf), file_->get())) > 0)
    verifier.VerifyUpdate(buf, base::checked_cast<int>(len));
  if (!verifier.VerifyFinal())
    return false;

  return true;
}

bool XPKPackage::ExtractToTemporaryDir(base::FilePath* target_path) {
  if (is_extracted_) {
    *target_path = temp_dir_.path();
    return true;
  }

  if (!IsValid()) {
    LOG(ERROR) << "The XPK file is not valid.";
    return false;
  }

  return Package::ExtractToTemporaryDir(target_path);
}

}  // namespace application
}  // namespace xwalk
