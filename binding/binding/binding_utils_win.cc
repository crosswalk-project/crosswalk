// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/binding/binding/binding_utils.h"

#include <windows.h>
#include <shlobj.h>

#include "base/file_util.h"
#include "base/strings/sys_string_conversions.h"

namespace xwalk {
namespace utils {

base::Time GetLastModifiedTime(const base::FilePath& path) {
  // On Windows, call CreatePlatformFile to open a directory will fail.
  // Because It's need FILE_FLAG_BACKUP_SEMANTICS flag to open a directory
  // by calling CreateFile WIN32 API. So use FileEnumerator to workaround.
  file_util::FileEnumerator::FileType type =
      static_cast<file_util::FileEnumerator::FileType>(
          file_util::FileEnumerator::FILES |
          file_util::FileEnumerator::DIRECTORIES);
  file_util::FileEnumerator files(
      path.DirName(), false, type, path.BaseName().value());
  if (files.Next() == path) {
    file_util::FileEnumerator::FindInfo info;
    files.GetFindInfo(&info);
    return files.GetLastModifiedTime(info);
  }

  return base::Time();
}

base::FilePath GetHomeDirectory() {
  base::FilePath dir;

  wchar_t* buf = new wchar_t[MAX_PATH];
  if (SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL,
                                  SHGFP_TYPE_CURRENT, buf)))
    dir = base::FilePath(buf);
  delete[] buf;

  return dir;
}

base::FilePath::StringType GetEnvironmentString(
    const base::FilePath::StringType& name) {
  base::FilePath::StringType value;

  DWORD size = ::GetEnvironmentVariable(name.c_str(), NULL, 0);
  if (size > 0) {
    // Get path from environment
    wchar_t* buf = new wchar_t[size];
    ::GetEnvironmentVariable(name.c_str(), buf, size);
    value.assign(buf);
    delete[] buf;
  }

  return value;
}

base::FilePath::StringType ExpandEnvironmentString(
    const base::FilePath::StringType& string) {
  base::FilePath::StringType value;

  DWORD size = ::ExpandEnvironmentStrings(string.c_str(), NULL, 0);
  if (size > 0) {
    wchar_t* buf = new(wchar_t[size]);
    ::ExpandEnvironmentStrings(string.c_str(), buf, size);
    value.assign(buf);
    delete[] buf;
  }

  return value;
}

std::string GetSystemLocale() {
  wchar_t locale[LOCALE_NAME_MAX_LENGTH];
  ::GetUserDefaultLocaleName(locale, sizeof(locale));
  return base::SysWideToUTF8(locale);
}

}  // namespace utils
}  // namespace xwalk
