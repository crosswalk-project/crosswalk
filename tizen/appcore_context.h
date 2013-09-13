// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_APPCORE_CONTEXT_H_
#define XWALK_TIZEN_APPCORE_CONTEXT_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"

namespace tizen {

// AppcoreContext makes Tizen task switcher be able to control xwalk.
class AppcoreContext {
 public:
  static scoped_ptr<AppcoreContext> Create();
  virtual ~AppcoreContext() {}
};

}  // namespace tizen

#endif  // XWALK_TIZEN_APPCORE_CONTEXT_H_
