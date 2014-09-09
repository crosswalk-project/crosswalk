// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/encryption_tizen.h"

#include "base/files/file_path.h"
#include "base/strings/string_util.h"

namespace xwalk {
namespace application {

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

bool EncryptData(const char* plain_data, int len, std::string* encrypted_data) {
  // TODO(yejingfu):
  // The encryption & decryption should be performed at system-level, such as
  // the trust-zone on ARM or chaabi on IA. But at this moment Tizen does
  // not include such security module.
  //
  // Other solution may be storing the security key within system-level storage
  // like gnome-keyring or some other external / internal storage. See
  // "src/components/os_crypt" for reference.
  // However this is still not supported in Tizen.
  //
  // So leave the EncryptData() and DecryptData() not implemented at present
  // until we get supports from system (low-level) in the future.
  //

  // Assume the input is valid (not empty).
  DCHECK(plain_data);
  DCHECK_GT(len, 0);
  DCHECK(encrypted_data);
  *encrypted_data = std::string(plain_data, len);
  return true;
}

bool DecryptData(const char* encrypted_data, int len, std::string* plain_data) {
  // TODO(yejingfu): See the comments in EncryptData().

  // Assume the input is valid (not empty).
  DCHECK(encrypted_data);
  DCHECK_GT(len, 0);
  DCHECK(plain_data);

  *plain_data = std::string(encrypted_data, len);
  return true;
}

}  // namespace application
}  // namespace xwalk
