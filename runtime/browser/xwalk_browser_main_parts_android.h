// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_ANDROID_H_
#define XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_ANDROID_H_

#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"

namespace xwalk {

class XWalkBrowserMainPartsAndroid : public XWalkBrowserMainParts {
 public:
  explicit XWalkBrowserMainPartsAndroid(
      const content::MainFunctionParams& parameters);
  virtual ~XWalkBrowserMainPartsAndroid();

  virtual void PreEarlyInitialization() OVERRIDE;
  virtual void PreMainMessageLoopStart() OVERRIDE;
  virtual void PostMainMessageLoopStart() OVERRIDE;
  virtual void PreMainMessageLoopRun() OVERRIDE;
  virtual void PostMainMessageLoopRun() OVERRIDE;

  virtual void RegisterInternalExtensionsInExtensionThreadServer(
      extensions::XWalkExtensionServer* server) OVERRIDE;

  // XWalkExtensionAndroid needs to register its extensions on
  // XWalkBrowserMainParts so they get correctly registered on-demand
  // by XWalkExtensionService each time a in_process Server is created.
  void RegisterExtension(scoped_ptr<extensions::XWalkExtension> extension);
  void UnregisterExtension(scoped_ptr<extensions::XWalkExtension> extension);

 private:
  ScopedVector<extensions::XWalkExtension> extensions_;

  DISALLOW_COPY_AND_ASSIGN(XWalkBrowserMainPartsAndroid);
};

inline XWalkBrowserMainPartsAndroid* ToAndroidMainParts(XWalkBrowserMainParts*
                                                        parts) {
    return static_cast<XWalkBrowserMainPartsAndroid*>(parts);
}

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_ANDROID_H_
