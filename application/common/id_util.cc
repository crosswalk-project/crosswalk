// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/id_util.h"

#include "base/files/file_path.h"
#include "base/strings/string_util.h"
#include "base/strings/string_number_conversions.h"
#include "crypto/sha2.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/tizen_application_handler.h"

#if defined(OS_TIZEN)
#include "xwalk/application/common/tizen/package_query.h"

#include "third_party/re2/re2/re2.h"
#endif

namespace xwalk {
namespace application {
namespace {
#if defined(OS_TIZEN)
const char kWGTAppIdPattern[] = "\\A([0-9a-zA-Z]{10})[.][0-9a-zA-Z]{1,52}\\z";
const char kXPKAppIdPattern[] = "\\Axwalk[.]([a-p]{32})\\z";
const char kPkgIdPattern[] = "\\A([0-9a-zA-Z]{10,})\\z";
const std::string kAppIdPrefix("xwalk.");
#endif
const size_t kIdSize = 16;
}  // namespace

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

std::string GenerateId(const std::string& input) {
  uint8 hash[kIdSize];
  crypto::SHA256HashString(input, hash, sizeof(hash));
  std::string output =
      base::StringToLowerASCII(base::HexEncode(hash, sizeof(hash)));
  ConvertHexadecimalToIDAlphabet(&output);

#if defined(OS_TIZEN)
  return kAppIdPrefix + output;
#else
  return output;
#endif
}

std::string GenerateIdForPath(const base::FilePath& path) {
  std::string path_bytes =
      std::string(reinterpret_cast<const char*>(path.value().data()),
                  path.value().size() * sizeof(base::FilePath::CharType));
  return GenerateId(path_bytes);
}

#if defined(OS_TIZEN)
bool IsValidWGTID(const std::string& id) {
  return RE2::FullMatch(id, kWGTAppIdPattern);
}

bool IsValidXPKID(const std::string& id) {
  return RE2::FullMatch(id, kXPKAppIdPattern);
}

bool IsValidPkgID(const std::string& id) {
  return RE2::FullMatch(id, kPkgIdPattern);
}

std::string AppIdToPkgId(const std::string& id) {
  std::string package_id;
  if (!RE2::FullMatch(id, kWGTAppIdPattern, &package_id) &&
      !RE2::FullMatch(id, kXPKAppIdPattern, &package_id)) {
    LOG(ERROR) << "Cannot get package_id from invalid app id";
    return std::string();
  }
  return package_id;
}

std::string PkgIdToAppId(const std::string& id) {
  base::FilePath app_path = GetPackagePath(id);
  if (app_path.empty())
    return std::string();

  return app_path.BaseName().value();
}

#endif

bool IsValidApplicationID(const std::string& id) {
#if defined(OS_TIZEN)
  return (IsValidWGTID(id) || IsValidXPKID(id));
#endif

  std::string temp = base::StringToLowerASCII(id);
  // Verify that the id is legal.
  if (temp.size() != (kIdSize * 2))
    return false;

  // We only support lowercase IDs, because IDs can be used as URL components
  // (where GURL will lowercase it).
  for (size_t i = 0; i < temp.size(); ++i)
    if (temp[i] < 'a' || temp[i] > 'p')
      return false;

  return true;
}

}  // namespace application
}  // namespace xwalk
