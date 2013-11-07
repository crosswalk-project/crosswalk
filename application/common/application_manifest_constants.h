// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_APPLICATION_MANIFEST_CONSTANTS_H_
#define XWALK_APPLICATION_COMMON_APPLICATION_MANIFEST_CONSTANTS_H_

// Keys used in JSON representation of applications.
namespace xwalk {
namespace application_manifest_keys {
  extern const char kAppKey[];
  extern const char kAppMainKey[];
  extern const char kAppMainScriptsKey[];
  extern const char kAppMainSourceKey[];
  extern const char kDescriptionKey[];
  extern const char kLaunchLocalPathKey[];
  extern const char kLaunchWebURLKey[];
  extern const char kManifestVersionKey[];
  extern const char kNameKey[];
  extern const char kPermissionsKey[];
  extern const char kVersionKey[];
  extern const char kWebURLsKey[];
}  // namespace application_manifest_keys

namespace application_manifest_errors {
  extern const char kInvalidDescription[];
  extern const char kInvalidKey[];
  extern const char kInvalidManifestVersion[];
  extern const char kInvalidName[];
  extern const char kInvalidVersion[];
  extern const char kManifestParseError[];
  extern const char kManifestUnreadable[];
  extern const char kPlatformAppNeedsManifestVersion2[];
}  // namespace application_manifest_errors
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_APPLICATION_MANIFEST_CONSTANTS_H_
