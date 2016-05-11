// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_ANDROID_H_
#define XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_ANDROID_H_

#include <string>
#include <vector>
#include "base/memory/ref_counted.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"

namespace xwalk {

class XWalkBrowserMainPartsAndroid : public XWalkBrowserMainParts {
 public:
  explicit XWalkBrowserMainPartsAndroid(
      const content::MainFunctionParams& parameters);
  ~XWalkBrowserMainPartsAndroid() override;

  void PreEarlyInitialization() override;
  void PreMainMessageLoopStart() override;
  void PostMainMessageLoopStart() override;
  void PreMainMessageLoopRun() override;
  void PostMainMessageLoopRun() override;

  void CreateInternalExtensionsForExtensionThread(
      content::RenderProcessHost* host,
      extensions::XWalkExtensionVector* extensions) override;

  // XWalkExtensionAndroid needs to register its extensions on
  // XWalkBrowserMainParts so they get correctly registered on-demand
  // by XWalkExtensionService each time a in_process Server is created.
  void RegisterExtension(scoped_ptr<extensions::XWalkExtension> extension);

  // Lookup the extension with the given name from the extension list that is
  // already registered. Returns NULL if no such extension exists.
  extensions::XWalkExtension* LookupExtension(const std::string& name);

  void RegisterExtensionInPath(const std::string& path);

 private:
  extensions::XWalkExtensionVector extensions_;

  DISALLOW_COPY_AND_ASSIGN(XWalkBrowserMainPartsAndroid);
};

inline XWalkBrowserMainPartsAndroid* ToAndroidMainParts(XWalkBrowserMainParts*
                                                        parts) {
    return static_cast<XWalkBrowserMainPartsAndroid*>(parts);
}

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_ANDROID_H_
