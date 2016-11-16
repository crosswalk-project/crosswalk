// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/common/xwalk_switches.h"

namespace switches {

// Specifies the icon file for the app window.
const char kAppIcon[] = "app-icon";

// Disables the usage of Portable Native Client.
const char kDisablePnacl[] = "disable-pnacl";

// Forces the maximum disk space to be used by the disk cache, in bytes.
const char kDiskCacheSize[] = "disk-cache-size";

// Enable all the experimental features in XWalk.
const char kExperimentalFeatures[] = "enable-xwalk-experimental-features";

// List the command lines feature flags.
const char kListFeaturesFlags[] = "list-features-flags";

const char kXWalkAllowExternalExtensionsForRemoteSources[] =
    "allow-external-extensions-for-remote-sources";

// Specifies the data path directory, which XWalk runtime will look for its
// state, e.g. cache, localStorage etc.
const char kXWalkDataPath[] = "data-path";

#if !defined(OS_ANDROID)
// Specifies if remote inspector can be opened when right clicking on the
// application.
const char kXWalkEnableInspector[] = "enable-inspector";

// Specifies if XWalk should disable form data completion.
const char kXWalkDisableSaveFormData[] = "disable-save-form-data";
#endif

#if defined(OS_ANDROID)
// Specifies the separated folder to save user data on Android.
const char kXWalkProfileName[] = "profile-name";
#endif

// By default, an https page cannot run JavaScript, CSS or plug-ins from http
// URLs. This provides an override to get the old insecure behavior.
const char kAllowRunningInsecureContent[]   = "allow-running-insecure-content";

// By default, an https page can load images, fonts or frames from an http
// page. This switch overrides this to block this lesser mixed-content problem.
const char kNoDisplayingInsecureContent[]   = "no-displaying-insecure-content";

#if defined(ENABLE_PLUGINS)
// Use the PPAPI (Pepper) Flash found at the given path.
const char kPpapiFlashPath[] = "ppapi-flash-path";

// Report the given version for the PPAPI (Pepper) Flash. The version should be
// numbers separated by '.'s (e.g., "12.3.456.78"). If not specified, it
// defaults to "10.2.999.999".
const char kPpapiFlashVersion[] = "ppapi-flash-version";
#endif

// Specifies the user data directory, which is where the browser will look for
// all of its state.
const char kUserDataDir[] = "user-data-dir";

// Overrides per-origin quota settings to unlimited storage for all
// apps/origins.
const char kUnlimitedStorage[] = "unlimited-storage";

}  // namespace switches
