// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/binding/binding/binding_utils.h"

#include <locale.h>
#include <stdlib.h>
#include <string>

#include "base/file_util.h"

namespace xwalk {
namespace utils {

base::Time GetLastModifiedTime(const base::FilePath& path) {
  base::PlatformFileError error;
  base::PlatformFile file = base::CreatePlatformFile(path,
  base::PLATFORM_FILE_OPEN | base::PLATFORM_FILE_READ, NULL, &error);
  if (error == base::PLATFORM_FILE_OK) {
    base::PlatformFileInfo info;
    base::GetPlatformFileInfo(file, &info);
    base::ClosePlatformFile(file);
    return info.last_modified;
  }
  return base::Time();
}

base::FilePath GetHomeDirectory() {
  return file_util::GetHomeDir();
}

base::FilePath::StringType GetEnvironmentString(
    const base::FilePath::StringType& name) {
  base::FilePath::StringType value;

  const char *buf = getenv(name.c_str());
  if (buf)
    value.assign(buf);

  return value;
}

base::FilePath::StringType ExpandEnvironmentString(
    const base::FilePath::StringType& string) {
  return string;
}

std::string GetSystemLocale() {
  std::string locale;
  setlocale(LC_ALL, "");
  locale.assign(setlocale(LC_MESSAGES, NULL));
  if (locale == "C" || locale == "POSIX")
    return "en-US";

  size_t pos;
  pos = locale.find('.');
  if (pos != std::string::npos)
    locale.erase(pos);
  pos = locale.find('_');
  if (pos != std::string::npos)
    locale[pos] = '-';
  return locale;
}

}  // namespace utils
}  // namespace xwalk
