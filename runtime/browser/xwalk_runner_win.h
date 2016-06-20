// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_RUNNER_WIN_H_
#define XWALK_RUNTIME_BROWSER_XWALK_RUNNER_WIN_H_

#include <string>

#include "xwalk/runtime/browser/xwalk_runner.h"


namespace xwalk {

class XWalkRunnerWin : public XWalkRunner {
 public:
  XWalkRunnerWin();

 protected:
  void CreateComponents() override;
  void InitializeEnvironmentVariablesForGoogleAPIs(
      content::RenderProcessHost* host) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkRunnerWin);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_RUNNER_WIN_H_
