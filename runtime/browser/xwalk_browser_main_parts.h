// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_H_
#define XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_H_

#include <string>
#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
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

namespace xwalk {

class RuntimeContext;
class RemoteDebuggingServer;
class XWalkRunner;

namespace extensions {
class XWalkExtensionService;
}

class XWalkBrowserMainParts : public content::BrowserMainParts {
 public:
  explicit XWalkBrowserMainParts(
      const content::MainFunctionParams& parameters);

  virtual ~XWalkBrowserMainParts();

  // BrowserMainParts overrides.
  virtual void PreEarlyInitialization() OVERRIDE;
  virtual int PreCreateThreads() OVERRIDE;
  virtual void PreMainMessageLoopStart() OVERRIDE;
  virtual void PostMainMessageLoopStart() OVERRIDE;
  virtual void PreMainMessageLoopRun() OVERRIDE;
  virtual bool MainMessageLoopRun(int* result_code) OVERRIDE;
  virtual void PostMainMessageLoopRun() OVERRIDE;

  // Create all the extensions to be hooked into a new
  // RenderProcessHost. Base class implementation should be called by
  // subclasses overriding this..
  virtual void CreateInternalExtensionsForUIThread(
      content::RenderProcessHost* host,
      extensions::XWalkExtensionVector* extensions);
  virtual void CreateInternalExtensionsForExtensionThread(
      content::RenderProcessHost* host,
      extensions::XWalkExtensionVector* extensions);

#if defined(OS_ANDROID)
  RuntimeContext* runtime_context() { return runtime_context_; }

  // XWalkExtensionAndroid needs to register its extensions on
  // XWalkBrowserMainParts so they get correctly registered on-demand
  // by XWalkExtensionService each time a in_process Server is created.
  void RegisterExtension(scoped_ptr<extensions::XWalkExtension> extension);
  void UnregisterExtension(scoped_ptr<extensions::XWalkExtension> extension);
#endif

 protected:
  void RegisterExternalExtensions();

  XWalkRunner* xwalk_runner_;

  RuntimeContext* runtime_context_;

  extensions::XWalkExtensionService* extension_service_;

  // Should be about:blank If no URL is specified in command line arguments.
  GURL startup_url_;

  // The main function parameters passed to BrowserMain.
  const content::MainFunctionParams& parameters_;

  // True if we need to run the default message loop defined in content.
  bool run_default_message_loop_;

  // Remote debugger server.
  scoped_ptr<RemoteDebuggingServer> remote_debugging_server_;

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkBrowserMainParts);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_H_
