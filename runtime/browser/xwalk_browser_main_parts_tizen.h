// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_TIZEN_H_
#define XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_TIZEN_H_

#include <string>
#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"

namespace xwalk {

class XWalkBrowserMainPartsTizen : public XWalkBrowserMainParts {
 public:
  explicit XWalkBrowserMainPartsTizen(
          const content::MainFunctionParams& parameters);
  virtual ~XWalkBrowserMainPartsTizen() {}

  virtual void PreMainMessageLoopStart() OVERRIDE;
  virtual void PreMainMessageLoopRun() OVERRIDE;

  virtual void RegisterInternalExtensionsInExtensionThreadServer(
      extensions::XWalkExtensionServer* server) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkBrowserMainPartsTizen);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_TIZEN_H_
