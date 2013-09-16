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

// static
scoped_ptr<XPKPackage> XPKPackage::Create(const base::FilePath& path) {
  if (!file_util::PathExists(path))
    scoped_ptr<XPKPackage>();
  scoped_ptr<ScopedStdioHandle> file(
      new ScopedStdioHandle(file_util::OpenFile(path, "rb")));
  Header header;
  size_t len = fread(&header, 1, sizeof(header), file->get());
  if (len < sizeof(header))
    return scoped_ptr<XPKPackage>();
  if (!strncmp(XPKPackage::kXPKPackageHeaderMagic,
               header.magic,
               sizeof(header.magic)) &&
      header.key_size > 0 &&
      header.key_size <= XPKPackage::kMaxPublicKeySize &&
      header.signature_size > 0 &&
      header.signature_size <= XPKPackage::kMaxSignatureKeySize) {
    scoped_ptr<XPKPackage> package(new XPKPackage(header, file.release()));
    if (package->IsOk())
      return package.Pass();
  }
  return scoped_ptr<XPKPackage>();
}

XPKPackage::XPKPackage(Header header, ScopedStdioHandle* file)
    : header_(header),
      file_(file),
      is_ok_(true) {
  zip_addr_ = sizeof(header) + header.key_size + header.signature_size;
  fseek(file_->get(), sizeof(header), SEEK_SET);
  key_.resize(header_.key_size);
  size_t len = fread(
      &key_.front(), sizeof(uint8), header_.key_size, file_->get());
  if (len < header_.key_size)
    is_ok_ = false;

  signature_.resize(header_.signature_size);
  len = fread(&signature_.front(),
              sizeof(uint8),
              header_.signature_size,
              file_->get());
  if (len < header_.signature_size)
    is_ok_ = false;

  if (!Validate())
    is_ok_ = false;

  std::string public_key =
      std::string(reinterpret_cast<char*>(&key_.front()), key_.size());
  id_ = GenerateId(public_key);
}

XPKPackage::~XPKPackage() {
}

bool XPKPackage::Validate() {
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
