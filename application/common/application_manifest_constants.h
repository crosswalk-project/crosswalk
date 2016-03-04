// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_APPLICATION_MANIFEST_CONSTANTS_H_
#define XWALK_APPLICATION_COMMON_APPLICATION_MANIFEST_CONSTANTS_H_

#include "xwalk/application/common/manifest.h"

// Keys used in JSON representation of applications.
namespace xwalk {
namespace application_manifest_keys {
  // Official fields (ordered as spec):

  extern const char kNameKey[];
  // extern const char kShortName[];
  // extern const char kIcons[];
  extern const char kDisplay[];
  extern const char kOrientationKey[];
  extern const char kStartURLKey[];
  extern const char kScopeKey[];
  extern const char kCSPKey[];
  extern const char kBoundsKey[];
  extern const char kWidthKey[];
  extern const char kHeightKey[];
  extern const char kMinWidthKey[];
  extern const char kMinHeightKey[];
  extern const char kMaxWidthKey[];
  extern const char kMaxHeightKey[];

  // Deprecated fields:

  extern const char kAppKey[];
  extern const char kLaunchWebURLKey[];
  extern const char kLaunchLocalPathKey[];
  extern const char kDeprecatedURLKey[];
  extern const char kDeprecatedVersionKey[];
  extern const char kDeprecatedDescriptionKey[];
  extern const char kDeprecatedCSPKey[];
  extern const char kLaunchScreen[];
  extern const char kLaunchScreenDefault[];
  extern const char kLaunchScreenImageBorderDefault[];
  extern const char kLaunchScreenImageBorderLandscape[];
  extern const char kLaunchScreenImageBorderPortrait[];
  extern const char kLaunchScreenLandscape[];
  extern const char kLaunchScreenPortrait[];
  extern const char kLaunchScreenReadyWhen[];
  extern const char kView[];
  extern const char kViewBackgroundColor[];

  // XWalk View extensions:
  extern const char kXWalkView[];
  extern const char kXWalkViewBackgroundColor[];

  // XWalk extensions:

  extern const char kXWalkPackageId[];
  extern const char kPermissionsKey[];
  extern const char kXWalkVersionKey[];
  extern const char kXWalkDescriptionKey[];
  extern const char kXWalkHostsKey[];
  extern const char kXWalkLaunchScreen[];
  extern const char kXWalkLaunchScreenDefault[];
  extern const char kXWalkLaunchScreenImageBorderDefault[];
  extern const char kXWalkLaunchScreenImageBorderLandscape[];
  extern const char kXWalkLaunchScreenImageBorderPortrait[];
  extern const char kXWalkLaunchScreenLandscape[];
  extern const char kXWalkLaunchScreenPortrait[];
  extern const char kXWalkLaunchScreenReadyWhen[];

  // Windows specific:
  extern const char kXWalkWindowsUpdateID[];
}  // namespace application_manifest_keys

namespace application_widget_keys {
  extern const char kNamespaceKey[];
  extern const char kXmlLangKey[];
  extern const char kDefaultLocaleKey[];
  extern const char kNameKey[];
  extern const char kLaunchLocalPathKey[];
  extern const char kWebURLsKey[];
  extern const char kWidgetKey[];
  extern const char kVersionKey[];
  extern const char kViewModesKey[];
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
  extern const char kWidgetNamespaceKey[];
  extern const char kWidgetNamespacePrefix[];
}  // namespace application_widget_keys

namespace application_manifest_values {
extern const char kDisplayModeFullscreen[];
extern const char kDisplayModeStandalone[];
extern const char kDisplayModeMinimalUI[];
extern const char kDisplayModeBrowser[];
}  // namespace application_manifest_values

namespace application_manifest_errors {
  extern const char kInvalidDescription[];
  extern const char kInvalidKey[];
  extern const char kInvalidName[];
  extern const char kInvalidVersion[];
  extern const char kManifestParseError[];
  extern const char kManifestUnreadable[];
}  // namespace application_manifest_errors

namespace application {
const char* GetNameKey(Manifest::Type type);
const char* GetCSPKey(Manifest::Type type);
}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_APPLICATION_MANIFEST_CONSTANTS_H_
