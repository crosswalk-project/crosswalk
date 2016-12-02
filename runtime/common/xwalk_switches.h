// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_COMMON_XWALK_SWITCHES_H_
#define XWALK_RUNTIME_COMMON_XWALK_SWITCHES_H_

#include "build/build_config.h"

// Defines all command line switches for XWalk.
namespace switches {

extern const char kAppIcon[];
extern const char kDisablePnacl[];
extern const char kDiskCacheSize[];
extern const char kExperimentalFeatures[];
extern const char kListFeaturesFlags[];
extern const char kXWalkAllowExternalExtensionsForRemoteSources[];
extern const char kXWalkDataPath[];
#if !defined(OS_ANDROID)
extern const char kXWalkEnableInspector[];
extern const char kXWalkDisableSaveFormData[];
#endif
extern const char kAllowRunningInsecureContent[];
extern const char kNoDisplayingInsecureContent[];

#if defined(OS_ANDROID)
extern const char kXWalkProfileName[];
#endif

#if defined(ENABLE_PLUGINS)
extern const char kPpapiFlashPath[];
extern const char kPpapiFlashVersion[];
#endif

extern const char kUserDataDir[];

extern const char kUnlimitedStorage[];

}  // namespace switches

#endif  // XWALK_RUNTIME_COMMON_XWALK_SWITCHES_H_
