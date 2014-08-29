// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_TIZEN_ENCRYPTION_H_
#define XWALK_APPLICATION_COMMON_TIZEN_ENCRYPTION_H_

#include <string>

namespace base {
class FilePath;
}

namespace xwalk {
namespace application {

bool RequiresEncryption(const base::FilePath& file_path);

bool EncryptData(const char* plain_data, int len,
                 const std::string& key, std::string* out_encrypted_data);

bool DecryptData(const char* encrypted_data, int len,
                 const std::string& key, std::string* out_plain_data);

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_TIZEN_ENCRYPTION_H_
