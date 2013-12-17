// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_RUNNER_H_
#define XWALK_RUNTIME_BROWSER_XWALK_RUNNER_H_

#include "base/memory/scoped_ptr.h"

namespace content {
class ContentBrowserClient;
}

class XWalkTestSuiteInitializer;

namespace xwalk {

// Main object for the Browser Process execution in Crosswalk. It is created and
// owned by XWalkMainDelegate. It's role is to own, setup and teardown all the
// subsystems of Crosswalk.
class XWalkRunner {
 public:
  static XWalkRunner* Get();
  virtual ~XWalkRunner();

 protected:
  XWalkRunner();

 private:
  friend class XWalkMainDelegate;
  friend class ::XWalkTestSuiteInitializer;

  // Create the XWalkRunner object. We use a factory function so that we can
  // switch the concrete class on compile time based on the platform, separating
  // the per-platform behavior and data in the subclasses.
  static scoped_ptr<XWalkRunner> Create();

  // Note: this is not public as we want to discourage the rest of Crosswalk to
  // rely directly on this object.
  content::ContentBrowserClient* GetContentBrowserClient();

  scoped_ptr<content::ContentBrowserClient> content_browser_client_;

  DISALLOW_COPY_AND_ASSIGN(XWalkRunner);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_RUNNER_H_
