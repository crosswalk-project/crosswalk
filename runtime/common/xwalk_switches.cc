// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/common/xwalk_switches.h"

namespace switches {

// Specifies the icon file for the app window.
const char kAppIcon[] = "app-icon";

// Disables the usage of Portable Native Client.
const char kDisablePnacl[] = "disable-pnacl";

// Enable all the experimental features in XWalk.
const char kExperimentalFeatures[] = "enable-xwalk-experimental-features";

// List the command lines feature flags.
const char kListFeaturesFlags[] = "list-features-flags";

const char kXWalkAllowExternalExtensionsForRemoteSources[] =
    "allow-external-extensions-for-remote-sources";

// Specifies the data path directory, which XWalk runtime will look for its
// state, e.g. cache, localStorage etc.
const char kXWalkDataPath[] = "data-path";

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

}  // namespace switches
