// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_BINDING_BINDING_BINDING_UTILS_H_
#define XWALK_BINDING_BINDING_BINDING_UTILS_H_

#include <string>

#include "base/files/file_path.h"
#include "base/time.h"

namespace xwalk {
namespace utils {

base::Time GetLastModifiedTime(const base::FilePath& path);

base::FilePath GetHomeDirectory();

base::FilePath::StringType GetEnvironmentString(
    const base::FilePath::StringType& name);
base::FilePath::StringType ExpandEnvironmentString(
    const base::FilePath::StringType& string);

std::string GetSystemLocale();

}  // namespace utils
}  // namespace xwalk

#endif  // XWALK_BINDING_BINDING_BINDING_UTILS_H_
