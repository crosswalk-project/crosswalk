// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_APPLICATION_MANIFEST_CONSTANTS_H_
#define XWALK_APPLICATION_COMMON_APPLICATION_MANIFEST_CONSTANTS_H_

#include <string>

#include "base/basictypes.h"
#include "googleurl/src/gurl.h"

// Keys used in JSON representation of applications.
namespace xwalk{
namespace application_manifest_keys {
  extern const char kApp[];
  extern const char kDescription[];
  extern const char kLaunchLocalPath[];
  extern const char kLaunchWebURL[];
  extern const char kManifestVersion[];
  extern const char kName[];
  extern const char kPlatformAppBackground[];
  extern const char kVersion[];
  extern const char kWebURLs[];
}  // namespace application_nanifest_keys

namespace application_manifest_errors {
  extern const char kInvalidDescription[];
  extern const char kInvalidKey[];
  extern const char kInvalidManifestVersion[];
  extern const char kInvalidName[];
  extern const char kInvalidVersion[];
  extern const char kManifestParseError[];
  extern const char kManifestUnreadable[];
  extern const char kPlatformAppNeedsManifestVersion2[];
}  // namespace application_manifest_errors
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_APPLICATION_MANIFEST_CONSTANTS_H_
