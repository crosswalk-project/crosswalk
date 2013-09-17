// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_CONSTANTS_H_
#define XWALK_APPLICATION_COMMON_CONSTANTS_H_

#include "base/files/file_path.h"

namespace xwalk {
namespace application {

// Scheme we serve application content from.
extern const char kApplicationScheme[];

// The name of the manifest inside an application.
extern const base::FilePath::CharType kManifestFilename[];

// The name of the messages file inside an application.
extern const base::FilePath::CharType kMessagesFilename[];

// The filename to use for main document generated from app.main.scripts.
extern const char kGeneratedMainDocumentFilename[];

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_CONSTANTS_H_
