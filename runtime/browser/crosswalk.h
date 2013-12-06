// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_CROSSWALK_H_
#define XWALK_RUNTIME_BROWSER_CROSSWALK_H_

#include "base/memory/scoped_ptr.h"

namespace xwalk {

// Main object for the Browser Process execution in Crosswalk. It is created and
// owned by XWalkMainDelegate. It's role is to setup (and teardown) all the
// subsystems of Crosswalk.
class Crosswalk {
 public:
  static Crosswalk* Get();
  virtual ~Crosswalk();

 protected:
  Crosswalk();

 private:
  friend class XWalkMainDelegate;

  // Create the Crosswalk object. We use a factory function so that we can
  // switch the concrete class on compile time based on the platform, separating
  // the per-platform behavior and data in the subclasses.
  static scoped_ptr<Crosswalk> Create();

  DISALLOW_COPY_AND_ASSIGN(Crosswalk);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_CROSSWALK_H_
