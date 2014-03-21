// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_APPLICATION_MANIFEST_CONSTANTS_H_
#define XWALK_APPLICATION_COMMON_APPLICATION_MANIFEST_CONSTANTS_H_

#include "xwalk/application/common/manifest.h"
// Keys used in JSON representation of applications.
namespace xwalk {
namespace application_manifest_keys {
  extern const char kAppKey[];
  extern const char kAppMainKey[];
  extern const char kAppMainScriptsKey[];
  extern const char kAppMainSourceKey[];
  extern const char kCSPKey[];
  extern const char kDescriptionKey[];
  extern const char kLaunchLocalPathKey[];
  extern const char kLaunchScreen[];
  extern const char kLaunchScreenReadyWhen[];
  extern const char kLaunchWebURLKey[];
  extern const char kManifestVersionKey[];
  extern const char kNameKey[];
  extern const char kPermissionsKey[];
  extern const char kURLKey[];
  extern const char kVersionKey[];
  extern const char kWebURLsKey[];

#if defined(OS_TIZEN)
  extern const char kTizenAppIdKey[];
  extern const char kIcon128Key[];
#endif
}  // namespace application_manifest_keys

namespace application_widget_keys {
  extern const char kNameKey[];
  extern const char kLaunchLocalPathKey[];
  extern const char kWebURLsKey[];
  extern const char kWidgetKey[];
  extern const char kVersionKey[];
  extern const char kAccessKey[];
  extern const char kAccessOriginKey[];
  extern const char kAccessSubdomainsKey[];
  extern const char kCSPKey[];
  extern const char kAuthorKey[];
  extern const char kDescriptionKey[];
  extern const char kShortNameKey[];
  extern const char kIDKey[];
  extern const char kAuthorEmailKey[];
  extern const char kAuthorHrefKey[];
  extern const char kHeightKey[];
  extern const char kWidthKey[];
  extern const char kPreferencesKey[];
  extern const char kPreferencesNameKey[];
  extern const char kPreferencesValueKey[];
  extern const char kPreferencesReadonlyKey[];
#if defined(OS_TIZEN)
  extern const char kTizenAppIdKey[];
  extern const char kIcon128Key[];
  extern const char kAllowNavigationKey[];
#endif
}  // namespace application_widget_keys

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

namespace application {

typedef application::Manifest Manifest;
const char* GetNameKey(Manifest::PackageType type);
const char* GetVersionKey(Manifest::PackageType type);
const char* GetWebURLsKey(Manifest::PackageType type);
const char* GetLaunchLocalPathKey(Manifest::PackageType type);
const char* GetCSPKey(Manifest::PackageType type);
#if defined(OS_TIZEN)
const char* GetTizenAppIdKey(Manifest::PackageType type);
const char* GetIcon128Key(Manifest::PackageType type);
#endif
}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_APPLICATION_MANIFEST_CONSTANTS_H_
