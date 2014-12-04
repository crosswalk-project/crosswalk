// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/tizen/encryption.h"

#include <string>

#include "base/files/file_util.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "crypto/encryptor.h"
#include "crypto/random.h"
#include "crypto/symmetric_key.h"

namespace xwalk {
namespace application {

const int kInitCounterSize = 16;

namespace {
// The following file types should be encrypted when the application is
// installed on the client device.
// The application launching, the file should be decrypted by run time.
const base::FilePath::StringType kHTMLFormat(FILE_PATH_LITERAL(".html"));
const base::FilePath::StringType kHTMFormat(FILE_PATH_LITERAL(".htm"));
const base::FilePath::StringType kJSFormat(FILE_PATH_LITERAL(".js"));
const base::FilePath::StringType kCSSFormat(FILE_PATH_LITERAL(".css"));
}

bool RequiresEncryption(const base::FilePath& file_path) {
  return EndsWith(file_path.value(), kHTMLFormat, false) ||
    EndsWith(file_path.value(), kHTMFormat, false) ||
    EndsWith(file_path.value(), kJSFormat, false) ||
    EndsWith(file_path.value(), kCSSFormat, false);
}

bool EncryptData(const char* plain_data, int len,
                 const std::string& key, std::string* out_encrypted_data) {
  // Assume the input is valid (not empty).
  DCHECK(plain_data);
  DCHECK_GT(len, 0);
  DCHECK(out_encrypted_data);

  // generate initial vector
  scoped_ptr<char[], base::FreeDeleter> initCounter =
      scoped_ptr<char[], base::FreeDeleter>(
          static_cast<char*>(malloc(kInitCounterSize)));
  crypto::RandBytes(initCounter.get(), kInitCounterSize);
  base::StringPiece init_counter_str(
      initCounter.get(), kInitCounterSize);

  scoped_ptr<crypto::SymmetricKey> sym_key(crypto::SymmetricKey::Import(
      crypto::SymmetricKey::AES, key));
  crypto::Encryptor encryptor;
  encryptor.Init(sym_key.get(), crypto::Encryptor::CTR, "");
  base::StringPiece plaintext_str(plain_data, len);
  encryptor.SetCounter(init_counter_str);
  std::string encrypted;
  bool result = encryptor.Encrypt(plaintext_str, &encrypted);
  if (!result)
    return result;
  *out_encrypted_data = std::string(initCounter.get(),
                                    kInitCounterSize) + encrypted;
  return result;
}

bool DecryptData(const char* encrypted_data, int len,
                 const std::string& key, std::string* out_plain_data) {
  // Assume the input is valid (not empty).
  DCHECK(encrypted_data);
  DCHECK_GT(len, 0);
  DCHECK(out_plain_data);

  std::string tmp_encrypted = std::string(encrypted_data, len);
  std::string init_counter = tmp_encrypted.substr(0, kInitCounterSize);
  std::string encrypted = tmp_encrypted.substr(kInitCounterSize,
                                               len-kInitCounterSize);

  scoped_ptr<crypto::SymmetricKey> sym_key(crypto::SymmetricKey::Import(
      crypto::SymmetricKey::AES, key));
  crypto::Encryptor encryptor;
  encryptor.Init(sym_key.get(), crypto::Encryptor::CTR, "");
  base::StringPiece init_counter_str(init_counter.c_str(),
                                     init_counter.size());
  encryptor.SetCounter(init_counter_str);
  return encryptor.Decrypt(encrypted, out_plain_data);
}

}  // namespace application
}  // namespace xwalk
