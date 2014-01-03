// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_runner_android.h"

namespace xwalk {

XWalkRunnerAndroid::XWalkRunnerAndroid() {}

XWalkRunnerAndroid::~XWalkRunnerAndroid() {}

// static
XWalkRunnerAndroid* XWalkRunnerAndroid::GetInstance() {
  return static_cast<XWalkRunnerAndroid*>(XWalkRunner::GetInstance());
}

}  // namespace xwalk
