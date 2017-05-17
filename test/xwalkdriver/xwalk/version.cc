// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/xwalkdriver/xwalk/version.h"

#include "base/strings/stringprintf.h"

namespace {

// This variable must be able to be found and parsed by the upload script.
const int kMinimumSupportedXwalkVersion[] = {29, 0, 1545, 0};

}  // namespace

const int kMinimumSupportedXwalkBuildNo = kMinimumSupportedXwalkVersion[2];

std::string GetMinimumSupportedXwalkVersion() {
  return base::StringPrintf(
      "%d.%d.%d.%d",
      kMinimumSupportedXwalkVersion[0],
      kMinimumSupportedXwalkVersion[1],
      kMinimumSupportedXwalkVersion[2],
      kMinimumSupportedXwalkVersion[3]);
}
