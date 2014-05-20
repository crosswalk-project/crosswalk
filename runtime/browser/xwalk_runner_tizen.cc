// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_runner_tizen.h"

#include "xwalk/runtime/browser/sysapps_component.h"
#include "xwalk/runtime/browser/xwalk_component.h"
#include "xwalk/runtime/common/xwalk_runtime_features.h"

namespace xwalk {

XWalkRunnerTizen::XWalkRunnerTizen() {}

XWalkRunnerTizen::~XWalkRunnerTizen() {}

// static
XWalkRunnerTizen* XWalkRunnerTizen::GetInstance() {
  return static_cast<XWalkRunnerTizen*>(XWalkRunner::GetInstance());
}

void XWalkRunnerTizen::PreMainMessageLoopRun() {
  XWalkRunner::PreMainMessageLoopRun();
}

std::string XWalkRunnerTizen::GetLocale() const {
  return tizen_locale_listener_.GetLocale();
}

}  // namespace xwalk
