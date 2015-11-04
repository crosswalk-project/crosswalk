// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_CONSTANTS_H_
#define XWALK_APPLICATION_COMMON_CONSTANTS_H_

#include "base/files/file_path.h"

namespace xwalk {
namespace application {

// Scheme we serve application content from.
extern const char kApplicationScheme[];

// The name of the manifest inside a XPK-packaged application.
extern const base::FilePath::CharType kManifestXpkFilename[];

// The name of the manifest inside a WGT-packaged application.
extern const base::FilePath::CharType kManifestWgtFilename[];

// The name of the messages file inside an application.
extern const base::FilePath::CharType kMessagesFilename[];

// The filename to use for main document generated from app.main.scripts.
extern const char kGeneratedMainDocumentFilename[];

// The name of cookies database file.
extern const base::FilePath::CharType kCookieDatabaseFilename[];

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_CONSTANTS_H_
