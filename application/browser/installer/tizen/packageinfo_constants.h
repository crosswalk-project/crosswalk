// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_INSTALLER_TIZEN_PACKAGEINFO_CONSTANTS_H_
#define XWALK_APPLICATION_BROWSER_INSTALLER_TIZEN_PACKAGEINFO_CONSTANTS_H_

#include "base/files/file_path.h"
#include "base/basictypes.h"

namespace xwalk {
namespace application_packageinfo_constants {
  extern const base::FilePath::CharType kAppDir[];
  extern const base::FilePath::CharType kAppDBPath[];
  extern const base::FilePath::CharType kAppDBJournalPath[];
  extern const base::FilePath::CharType kIconDir[];
  extern const base::FilePath::CharType kXmlDir[];
  extern const base::FilePath::CharType kXwalkPath[];

  extern const base::FilePath::CharType kExecDir[];
  extern const char kDefaultIconName[];
  extern const char kIconKey[];
  extern const char kOwner[];

  extern const char kXmlExtension[];
  extern const char kSeparator[];
}  // namespace application_packageinfo_constants
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_INSTALLER_TIZEN_PACKAGEINFO_CONSTANTS_H_
