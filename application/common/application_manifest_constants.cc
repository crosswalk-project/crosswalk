// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace application_manifest_keys {

// Official fields (ordered as spec):

const char kNameKey[] = "name";
const char kDisplay[] = "display";
const char kStartURLKey[] = "start_url";
const char kCSPKey[] = "csp";

// Deprecated entries:

const char kAppKey[] = "app";
const char kLaunchLocalPathKey[] = "app.launch.local_path";
const char kLaunchWebURLKey[] = "app.launch.web_url";
const char kDeprecatedURLKey[] = "url";
const char kDeprecatedVersionKey[] = "version";
const char kDeprecatedDescriptionKey[] = "description";
const char kDeprecatedCSPKey[] = "content_security_policy";
const char kLaunchScreen[] = "launch_screen";
const char kLaunchScreenDefault[] = "launch_screen.default";
const char kLaunchScreenImageBorderDefault[] =
    "launch_screen.default.image_border";
const char kLaunchScreenImageBorderLandscape[] =
    "launch_screen.landscape.image_border";
const char kLaunchScreenImageBorderPortrait[] =
    "launch_screen.portrait.image_border";
const char kLaunchScreenLandscape[] =
    "launch_screen.landscape";
const char kLaunchScreenPortrait[] =
    "launch_screen.portrait";
const char kLaunchScreenReadyWhen[] =
    "launch_screen.ready_when";

// XWalk W3C Manifest (XPK) extensions:

const char kPermissionsKey[] = "permissions";
const char kXWalkVersionKey[] = "xwalk_version";
const char kXWalkDescriptionKey[] = "xwalk_description";
const char kXWalkHostsKey[] = "xwalk_hosts";
const char kXWalkLaunchScreen[] = "xwalk_launch_screen";
const char kXWalkLaunchScreenDefault[] = "xwalk_launch_screen.default";
const char kXWalkLaunchScreenImageBorderDefault[] =
    "xwalk_launch_screen.default.image_border";
const char kXWalkLaunchScreenImageBorderLandscape[] =
    "xwalk_launch_screen.landscape.image_border";
const char kXWalkLaunchScreenImageBorderPortrait[] =
    "xwalk_launch_screen.portrait.image_border";
const char kXWalkLaunchScreenLandscape[] =
    "xwalk_launch_screen.landscape";
const char kXWalkLaunchScreenPortrait[] =
    "xwalk_launch_screen.portrait";
const char kXWalkLaunchScreenReadyWhen[] =
    "xwalk_launch_screen.ready_when";

#if defined(OS_TIZEN)
const char kTizenAppIdKey[] = "tizen_app_id";
const char kIcon128Key[] = "icons.128";
const char kXWalkMediaAppClass[] = "xwalk_media_app_class";
#endif

}  // namespace application_manifest_keys

// manifest keys for widget applications.
namespace application_widget_keys {

const char kNamespaceKey[] = "@namespace";
const char kXmlLangKey[] = "@lang";
const char kDefaultLocaleKey[] = "widget.@defaultlocale";
const char kNameKey[] = "widget.name.#text";
const char kVersionKey[] = "widget.@version";
const char kViewModesKey[] = "widget.@viewmodes";
const char kWidgetKey[] = "widget";
const char kLaunchLocalPathKey[] = "widget.content.@src";
const char kWebURLsKey[] = "widget.@id";
const char kAuthorKey[] = "widget.author.#text";
const char kDescriptionKey[] = "widget.description.#text";
const char kShortNameKey[] = "widget.name.@short";
const char kIDKey[] = "widget.@id";
const char kAuthorEmailKey[] = "widget.author.@email";
const char kAuthorHrefKey[] = "widget.author.@href";
const char kHeightKey[] = "widget.@height";
const char kWidthKey[] = "widget.@width";
const char kPreferencesKey[] = "widget.preference";
const char kCSPKey[] = "widget.content-security-policy.#text";
const char kAccessKey[] = "widget.access";

// Child keys inside 'kPreferencesKey'.
const char kPreferencesNameKey[] = "@name";
const char kPreferencesValueKey[] = "@value";
const char kPreferencesReadonlyKey[] = "@readonly";

const char kWidgetNamespaceKey[] = "widget.@namespace";
const char kWidgetNamespacePrefix[] = "http://www.w3.org/ns/widgets";

// Child keys inside 'kAccessKey'.
const char kAccessOriginKey[] = "@origin";
const char kAccessSubdomainsKey[] = "@subdomains";

#if defined(OS_TIZEN)
const char kTizenWidgetKey[] = "widget";
const char kIcon128Key[] = "widget.icon.@src";
const char kTizenApplicationKey[] = "widget.application";
// Child keys inside 'kTizenApplicationKey'
const char kTizenApplicationIdKey[] = "@id";
const char kTizenApplicationPackageKey[] = "@package";
const char kTizenApplicationRequiredVersionKey[] = "@required_version";

const char kTizenAppIdKey[] = "widget.application.@package";
const char kAllowNavigationKey[] = "widget.allow-navigation.#text";
const char kCSPReportOnlyKey[] =
    "widget.content-security-policy-report-only.#text";
const char kTizenSettingKey[] = "widget.setting";
const char kTizenBackgroundSupportKey[] = "widget.setting.@background-support";
const char kTizenContextMenuKey[] = "widget.setting.@context-menu";
const char kTizenHardwareKey[] = "widget.setting.@hwkey-event";
const char kTizenEncryptionKey[] = "widget.setting.@encryption";
const char kTizenMetaDataKey[] = "widget.metadata";
// Child keys inside 'kTizenMetaDataKey'
const char kTizenMetaDataNameKey[] = "@key";
const char kTizenMetaDataValueKey[] = "@value";
const char kTizenSplashScreenKey[] = "widget.splash-screen";
const char kTizenSplashScreenSrcKey[] = "@src";
const char kContentNamespace[] = "widget.content.@namespace";
const char kTizenScreenOrientationKey[] = "widget.setting.@screen-orientation";
const char kTizenAppWidgetFullKey[] = "widget.app-widget";
const char kTizenAppWidgetKey[] = "app-widget";
const char kTizenAppWidgetIdKey[] = "@id";
const char kTizenAppWidgetPrimaryKey[] = "@primary";
const char kTizenAppWidgetUpdatePeriodKey[] = "@update-period";
const char kTizenAppWidgetAutoLaunchKey[] = "@auto-launch";
const char kTizenAppWidgetBoxLabelKey[] = "box-label";
const char kTizenAppWidgetBoxLabelLangKey[] = "@lang";
const char kTizenAppWidgetBoxLabelTextKey[] = "#text";
const char kTizenAppWidgetBoxIconKey[] = "box-icon";
const char kTizenAppWidgetBoxIconSrcKey[] = "@src";
const char kTizenAppWidgetBoxContentKey[] = "box-content";
const char kTizenAppWidgetBoxContentSrcKey[] = "@src";
const char kTizenAppWidgetBoxContentMouseEventKey[] = "@mouse-event";
const char kTizenAppWidgetBoxContentTouchEffectKey[] = "@touch-effect";
const char kTizenAppWidgetBoxContentSizeKey[] = "box-size";
const char kTizenAppWidgetBoxContentSizeTextKey[] = "#text";
const char kTizenAppWidgetBoxContentSizePreviewKey[] = "@preview";
const char kTizenAppWidgetBoxContentSizeUseDecorationKey[] = "@use-decoration";
const char kTizenAppWidgetBoxContentDropViewKey[] = "pd";
const char kTizenAppWidgetBoxContentDropViewSrcKey[] = "@src";
const char kTizenAppWidgetBoxContentDropViewWidthKey[] = "@width";
const char kTizenAppWidgetBoxContentDropViewHeightKey[] = "@height";
// App control
const char kTizenApplicationAppControlsKey[] = "widget.app-control";
const char kTizenApplicationAppControlSrcKey[] = "src";
const char kTizenApplicationAppControlOperationKey[] = "operation";
const char kTizenApplicationAppControlUriKey[] = "uri";
const char kTizenApplicationAppControlMimeKey[] = "mime";
const char kTizenApplicationAppControlChildNameAttrKey[] = "@name";
const char kTizenNamespacePrefix[] = "http://tizen.org/ns/widgets";
#endif

}  // namespace application_widget_keys

namespace application_manifest_errors {
const char kInvalidDescription[] =
    "Invalid value for 'description'.";
const char kInvalidKey[] =
    "Value 'key' is missing or invalid.";
const char kInvalidName[] =
    "Required value 'name' is missing or invalid.";
const char kInvalidVersion[] =
    "Required value 'version' is missing or invalid. It must be between 1-4 "
    "dot-separated integers each between 0 and 65536.";
const char kManifestParseError[] =
    "Manifest is not valid JSON.";
const char kManifestUnreadable[] =
    "Manifest file is missing or unreadable.";
}  // namespace application_manifest_errors

namespace application {

const char* GetNameKey(Manifest::Type manifest_type) {
  if (manifest_type == Manifest::TYPE_WIDGET)
    return application_widget_keys::kNameKey;

  return application_manifest_keys::kNameKey;
}

const char* GetVersionKey(Manifest::Type manifest_type) {
  if (manifest_type == Manifest::TYPE_WIDGET)
    return application_widget_keys::kVersionKey;

  return application_manifest_keys::kXWalkVersionKey;
}

const char* GetCSPKey(Manifest::Type manifest_type) {
  if (manifest_type == Manifest::TYPE_WIDGET)
    return application_widget_keys::kCSPKey;

  return application_manifest_keys::kCSPKey;
}

#if defined(OS_TIZEN)
const char* GetTizenAppIdKey(Manifest::Type manifest_type) {
  if (manifest_type == Manifest::TYPE_WIDGET)
    return application_widget_keys::kTizenAppIdKey;

  return application_manifest_keys::kTizenAppIdKey;
}

const char* GetIcon128Key(Manifest::Type manifest_type) {
  if (manifest_type == Manifest::TYPE_WIDGET)
    return application_widget_keys::kIcon128Key;

  return application_manifest_keys::kIcon128Key;
}
#endif
}  // namespace application
}  // namespace xwalk
