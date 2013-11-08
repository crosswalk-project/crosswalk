// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/common/xwalk_switches.h"

namespace switches {

// Specifies the data path directory, which XWalk runtime will look for its
// state, e.g. cache, localStorage etc.
const char kXWalkDataPath[] = "data-path";

// Specifies the icon file for the app window.
const char kAppIcon[] = "app-icon";

// Indicate crosswalk runtime will run as a service.
const char kRunAsService[] = "run-as-service";

// Specifies the window whether launched with fullscreen mode.
const char kFullscreen[] = "fullscreen";

// TODO(Bai): These switches should be removed after daemon mode is done.
// Specifies list all installed applications.
const char kListApplications[] = "list-apps";

// Specifies install an application.
const char kInstall[] = "install";

// Spedifies uninstall an application from runtime.
const char kUninstall[] = "uninstall";

// Specifies where XWalk will look for external extensions.
const char kXWalkExternalExtensionsPath[] = "external-extensions-path";

const char kXWalkAllowExternalExtensionsForRemoteSources[] =
    "allow-external-extensions-for-remote-sources";

}  // namespace switches
