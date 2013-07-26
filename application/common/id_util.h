// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_ID_UTIL_H_
#define XWALK_APPLICATION_COMMON_ID_UTIL_H_

#include <string>

namespace base {
class FilePath;
}

namespace xwalk_application {
namespace id_util {

// The number of bytes in a legal id.
extern const size_t kIdSize;

// Generates an application ID from arbitrary input. The same input string will
// always generate the same output ID.
std::string GenerateId(const std::string& input);

// Generate an ID for an application in the given path.
// Used while developing applications, before they have a key.
std::string GenerateIdForPath(const base::FilePath& path);

}  // namespace id_util
}  // namespace xwalk_application

#endif  // XWALK_APPLICATION_COMMON_ID_UTIL_H_
