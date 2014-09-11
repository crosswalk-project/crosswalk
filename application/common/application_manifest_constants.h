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
  // extern const char kOrientation[];
  extern const char kStartURLKey[];
  extern const char kCSPKey[];

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

  // XWalk extensions:

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

#if defined(OS_TIZEN)
  extern const char kTizenAppIdKey[];
  extern const char kIcon128Key[];
#endif
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
#if defined(OS_TIZEN)
  extern const char kTizenWidgetKey[];
  extern const char kTizenApplicationKey[];
  extern const char kTizenApplicationIdKey[];
  extern const char kTizenApplicationPackageKey[];
  extern const char kTizenApplicationRequiredVersionKey[];
  extern const char kTizenAppIdKey[];
  extern const char kIcon128Key[];
  extern const char kAllowNavigationKey[];
  extern const char kCSPReportOnlyKey[];
  extern const char kTizenSettingKey[];
  extern const char kTizenContextMenuKey[];
  extern const char kTizenHardwareKey[];
  extern const char kTizenEncryptionKey[];
  extern const char kTizenMetaDataKey[];
  extern const char kTizenMetaDataNameKey[];
  extern const char kTizenMetaDataValueKey[];
  extern const char kTizenSplashScreenKey[];
  extern const char kTizenSplashScreenSrcKey[];
  extern const char kContentNamespace[];
  extern const char kTizenScreenOrientationKey[];
  extern const char kTizenAppWidgetFullKey[];
  extern const char kTizenAppWidgetKey[];
  extern const char kTizenAppWidgetIdKey[];
  extern const char kTizenAppWidgetPrimaryKey[];
  extern const char kTizenAppWidgetUpdatePeriodKey[];
  extern const char kTizenAppWidgetAutoLaunchKey[];
  extern const char kTizenAppWidgetBoxLabelKey[];
  extern const char kTizenAppWidgetBoxLabelLangKey[];
  extern const char kTizenAppWidgetBoxLabelTextKey[];
  extern const char kTizenAppWidgetBoxIconKey[];
  extern const char kTizenAppWidgetBoxIconSrcKey[];
  extern const char kTizenAppWidgetBoxContentKey[];
  extern const char kTizenAppWidgetBoxContentSrcKey[];
  extern const char kTizenAppWidgetBoxContentMouseEventKey[];
  extern const char kTizenAppWidgetBoxContentTouchEffectKey[];
  extern const char kTizenAppWidgetBoxContentSizeKey[];
  extern const char kTizenAppWidgetBoxContentSizeTextKey[];
  extern const char kTizenAppWidgetBoxContentSizePreviewKey[];
  extern const char kTizenAppWidgetBoxContentSizeUseDecorationKey[];
  extern const char kTizenAppWidgetBoxContentDropViewKey[];
  extern const char kTizenAppWidgetBoxContentDropViewSrcKey[];
  extern const char kTizenAppWidgetBoxContentDropViewWidthKey[];
  extern const char kTizenAppWidgetBoxContentDropViewHeightKey[];
#endif
}  // namespace application_widget_keys

#if defined(OS_TIZEN)
extern const char kTizenNamespacePrefix[];
#endif

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
#if defined(OS_TIZEN)
const char* GetTizenAppIdKey(Manifest::Type type);
const char* GetIcon128Key(Manifest::Type type);
#endif
}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_APPLICATION_MANIFEST_CONSTANTS_H_
