// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_RUNNER_TIZEN_H_
#define XWALK_RUNTIME_BROWSER_XWALK_RUNNER_TIZEN_H_

#include <string>

#include "xwalk/runtime/browser/xwalk_runner.h"

namespace xwalk {

// Main object customizations for the Tizen port of Crosswalk. Any objects
// specific to Tizen should be created here.
class XWalkRunnerTizen : public XWalkRunner {
 public:
  // See documentation in xwalk_runner.h about when it is valid to access
  // XWalkRunner directly. Relying too much on this accessor makes code harder
  // to change and harder to reason about.
  static XWalkRunnerTizen* GetInstance();

  ~XWalkRunnerTizen() override;

  void PreMainMessageLoopRun() override;

 protected:
  void InitializeRuntimeVariablesForExtensions(
      const content::RenderProcessHost* host,
      base::ValueMap* variables) override;

 private:
  friend class XWalkRunner;
  XWalkRunnerTizen();
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_RUNNER_TIZEN_H_
