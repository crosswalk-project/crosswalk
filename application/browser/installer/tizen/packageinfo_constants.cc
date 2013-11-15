// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/installer/tizen/packageinfo_constants.h"

namespace xwalk {
namespace application_packageinfo_constants {

const base::FilePath::CharType kAppDir[] =
    FILE_PATH_LITERAL("applications");
const base::FilePath::CharType kAppDBPath[] =
    FILE_PATH_LITERAL("applications.db");
const base::FilePath::CharType kAppDBJournalPath[] =
    FILE_PATH_LITERAL("applications.db-journal");
const base::FilePath::CharType kIconDir[] =
    FILE_PATH_LITERAL("/opt/share/icons/default/small/");
const base::FilePath::CharType kXmlDir[] =
    FILE_PATH_LITERAL("/opt/share/packages/");
const base::FilePath::CharType kXwalkPath[] =
    FILE_PATH_LITERAL("/usr/lib/xwalk/xwalk");
const base::FilePath::CharType kExecDir[] =
    FILE_PATH_LITERAL("bin");

const char kDefaultIconName[] = "crosswalk.png";
const char kIconKey[] = "icons.128";
const char kOwner[] = "app";

const char kXmlExtension[] = ".xml";
const char kSeparator[] = ".";

}  // namespace application_packageinfo_constants
}  // namespace xwalk
