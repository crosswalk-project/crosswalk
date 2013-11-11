// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_MAC_H_
#define XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_MAC_H_

#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"

namespace xwalk {

class XWalkBrowserMainPartsMac : public XWalkBrowserMainParts {
 public:
  explicit XWalkBrowserMainPartsMac(
      const content::MainFunctionParams& parameters);
  virtual ~XWalkBrowserMainPartsMac() {}

  virtual void PreMainMessageLoopStart() OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkBrowserMainPartsMac);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_MAC_H_
