// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_RUNNER_TIZEN_H_
#define XWALK_RUNTIME_BROWSER_XWALK_RUNNER_TIZEN_H_

#include <cynara-client.h>
#include <string>

#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/browser/tizen/tizen_locale_listener.h"

namespace xwalk {

// Main object customizations for the Tizen port of Crosswalk. Any objects
// specific to Tizen should be created here.
class XWalkRunnerTizen : public XWalkRunner {
 public:
  // See documentation in xwalk_runner.h about when it is valid to access
  // XWalkRunner directly. Relying too much on this accessor makes code harder
  // to change and harder to reason about.
  static XWalkRunnerTizen* GetInstance();

  virtual ~XWalkRunnerTizen();

  virtual void PreMainMessageLoopRun() OVERRIDE;

  cynara* cynara_handler() { return  cynara_handler_; }

 private:
  friend class XWalkRunner;
  XWalkRunnerTizen();

  cynara* cynara_handler_;
  cynara_configuration* cynara_conf_;

  TizenLocaleListener tizen_locale_listener_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_RUNNER_TIZEN_H_
