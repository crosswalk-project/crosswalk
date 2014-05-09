// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/constants.h"

namespace xwalk {
namespace application {

// TODO(xiang): might rename this according to the spec.
const char kApplicationScheme[] = "app";
const base::FilePath::CharType kManifestXpkFilename[] =
    FILE_PATH_LITERAL("manifest.json");
const base::FilePath::CharType kManifestWgtFilename[] =
    FILE_PATH_LITERAL("config.xml");
const base::FilePath::CharType kMessagesFilename[] =
    FILE_PATH_LITERAL("messages.json");
const char kGeneratedMainDocumentFilename[] =
    "_generated_main_document.html";
const base::FilePath::CharType kCookieDatabaseFilename[] =
    FILE_PATH_LITERAL("ApplicationCookies");

}  // namespace application
}  // namespace xwalk
