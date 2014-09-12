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

// List the command lines feature flags.
const char kListFeaturesFlags[] = "list-features-flags";

const char kXWalkAllowExternalExtensionsForRemoteSources[] =
    "allow-external-extensions-for-remote-sources";

// Specifies the data path directory, which XWalk runtime will look for its
// state, e.g. cache, localStorage etc.
const char kXWalkDataPath[] = "data-path";

}  // namespace switches
