// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace application_manifest_keys {
const char kAppKey[] = "app";
const char kAppMainKey[] = "app.main";
const char kAppMainScriptsKey[] = "app.main.scripts";
const char kAppMainSourceKey[] = "app.main.source";
const char kDescriptionKey[] = "description";
const char kLaunchLocalPathKey[] = "app.launch.local_path";
const char kLaunchWebURLKey[] = "app.launch.web_url";
const char kManifestVersionKey[] = "manifest_version";
const char kNameKey[] = "name";
const char kPermissionsKey[] = "permissions";
const char kVersionKey[] = "version";
const char kWebURLsKey[] = "app.urls";
}  // namespace application_manifest_keys

namespace application_manifest_errors {
const char kInvalidDescription[] =
    "Invalid value for 'description'.";
const char kInvalidKey[] =
    "Value 'key' is missing or invalid.";
const char kInvalidManifestVersion[] =
    "Invalid value for 'manifest_version'. Must be an integer greater than "
    "zero.";
const char kInvalidName[] =
    "Required value 'name' is missing or invalid.";
const char kInvalidVersion[] =
    "Required value 'version' is missing or invalid. It must be between 1-4 "
    "dot-separated integers each between 0 and 65536.";
const char kManifestParseError[] =
    "Manifest is not valid JSON.";
const char kManifestUnreadable[] =
    "Manifest file is missing or unreadable.";
const char kPlatformAppNeedsManifestVersion2[] =
    "Packaged apps need manifest_version set to >= 2";
}  // namespace application_manifest_errors

}  // namespace xwalk
