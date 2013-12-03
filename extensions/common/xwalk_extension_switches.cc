// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension_switches.h"

namespace switches {

// TODO(cmarcelo): Currently we are disabling it by default, once Extension
// Process patches land, we'll change this to be "disable-extension-process"
// so that it will be enabled by default. Remember to forcely disable it for
// Android.
const char kXWalkDisableExtensionProcess[] =
    "disable-extension-process";

// Used internally to launch an extension process.
const char kXWalkExtensionProcess[] = "xwalk-extension-process";

}  // namespace switches
