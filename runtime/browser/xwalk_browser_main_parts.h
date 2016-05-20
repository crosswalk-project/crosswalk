// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_H_
#define XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_H_

#include <memory>
#include <string>

#include "base/memory/scoped_vector.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/common/main_function_params.h"
#include "url/gurl.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension_permission_types.h"
#include "xwalk/extensions/common/xwalk_extension_vector.h"

namespace content {
class RenderProcessHost;
}

#if defined(USE_AURA)
namespace wm {
class WMState;
}
#endif

namespace devtools_http_handler {
class DevToolsHttpHandler;
}

namespace xwalk {

class XWalkRunner;

namespace extensions {
class XWalkExtensionService;
}

class XWalkBrowserMainParts : public content::BrowserMainParts {
 public:
  explicit XWalkBrowserMainParts(
      const content::MainFunctionParams& parameters);

  ~XWalkBrowserMainParts() override;

  // BrowserMainParts overrides.
  void PreEarlyInitialization() override;
  int PreCreateThreads() override;
  void PreMainMessageLoopStart() override;
  void PostMainMessageLoopStart() override;
  void PreMainMessageLoopRun() override;
  bool MainMessageLoopRun(int* result_code) override;
  void PostMainMessageLoopRun() override;

  // Create all the extensions to be hooked into a new
  // RenderProcessHost. Base class implementation should be called by
  // subclasses overriding this..
  virtual void CreateInternalExtensionsForUIThread(
      content::RenderProcessHost* host,
      extensions::XWalkExtensionVector* extensions);
  virtual void CreateInternalExtensionsForExtensionThread(
      content::RenderProcessHost* host,
      extensions::XWalkExtensionVector* extensions);

  devtools_http_handler::DevToolsHttpHandler* devtools_http_handler() {
    return devtools_http_handler_.get();
  }

 protected:
  void RegisterExternalExtensions();

  XWalkRunner* xwalk_runner_;

  extensions::XWalkExtensionService* extension_service_;

  // Should be about:blank If no URL is specified in command line arguments.
  GURL startup_url_;

  // The main function parameters passed to BrowserMain.
  const content::MainFunctionParams& parameters_;

  // True if we need to run the default message loop defined in content.
  bool run_default_message_loop_;

  std::unique_ptr<devtools_http_handler::DevToolsHttpHandler> devtools_http_handler_;

 private:
#if defined(USE_AURA)
  std::unique_ptr<wm::WMState> wm_state_;
#endif
  DISALLOW_COPY_AND_ASSIGN(XWalkBrowserMainParts);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_H_
