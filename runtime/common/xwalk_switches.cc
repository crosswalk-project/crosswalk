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

// Specifies the window whether launched with fullscreen mode.
const char kFullscreen[] = "fullscreen";

// Specifies install an application.
const char kInstall[] = "install";

// Specifies list all installed applications.
const char kListApplications[] = "list-apps";

// List the command lines feature flags.
const char kListFeaturesFlags[] = "list-features-flags";

// Specifies uninstall an application from runtime.
const char kUninstall[] = "uninstall";

const char kXWalkAllowExternalExtensionsForRemoteSources[] =
    "allow-external-extensions-for-remote-sources";

// Runs Crosswalk in service mode: it loads no application by default but stays
// alive, and listens for external requests to launch applications. The way to
// issue these requests is platform-specific.
const char kXWalkRunAsService[] = "run-as-service";

// Specifies the data path directory, which XWalk runtime will look for its
// state, e.g. cache, localStorage etc.
const char kXWalkDataPath[] = "data-path";

}  // namespace switches
