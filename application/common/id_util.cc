// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/id_util.h"

#include "base/files/file_path.h"
#include "base/strings/string_util.h"
#include "base/strings/string_number_conversions.h"
#include "crypto/sha2.h"

namespace xwalk {
namespace application {

// Converts a normal hexadecimal string into the alphabet used by applications.
// We use the characters 'a'-'p' instead of '0'-'f' to avoid ever having a
// completely numeric host, since some software interprets that as an IP
// address.
static void ConvertHexadecimalToIDAlphabet(std::string* id) {
  for (size_t i = 0; i < id->size(); ++i) {
    int val;
    if (base::HexStringToInt(base::StringPiece(
            id->begin() + i, id->begin() + i + 1), &val)) {
      (*id)[i] = val + 'a';
    } else {
      (*id)[i] = 'a';
    }
  }
}

// First 16 bytes of SHA256 hashed public key.
const size_t kIdSize = 16;

std::string GenerateId(const std::string& input) {
  uint8 hash[kIdSize];
  crypto::SHA256HashString(input, hash, sizeof(hash));
  std::string output = StringToLowerASCII(base::HexEncode(hash, sizeof(hash)));
  ConvertHexadecimalToIDAlphabet(&output);

  return output;
}

std::string GenerateIdForPath(const base::FilePath& path) {
  std::string path_bytes =
      std::string(reinterpret_cast<const char*>(path.value().data()),
                  path.value().size() * sizeof(base::FilePath::CharType));
  return GenerateId(path_bytes);
}

}  // namespace application
}  // namespace xwalk
