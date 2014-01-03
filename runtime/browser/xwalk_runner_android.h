// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_RUNNER_ANDROID_H_
#define XWALK_RUNTIME_BROWSER_XWALK_RUNNER_ANDROID_H_

#include "xwalk/runtime/browser/xwalk_runner.h"

namespace xwalk {

// Main object customizations for the Android port of Crosswalk. Any objects
// specific to Android should be created here.
class XWalkRunnerAndroid : public XWalkRunner {
 public:
  // See documentation in xwalk_runner.h about when it is valid to access
  // XWalkRunner directly. Relying too much on this accessor makes code harder
  // to change and harder to reason about.
  static XWalkRunnerAndroid* GetInstance();

  virtual ~XWalkRunnerAndroid();

 private:
  friend class XWalkRunner;
  XWalkRunnerAndroid();
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_RUNNER_ANDROID_H_
