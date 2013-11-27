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

namespace xwalk {

namespace extensions {
class XWalkExtension;
class XWalkExtensionServer;
}

class RuntimeContext;
class RuntimeRegistry;
class RemoteDebuggingServer;

class XWalkBrowserMainParts : public content::BrowserMainParts,
    public extensions::XWalkExtensionService::Delegate {
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

  // XWalkExtensionService::Delegate overrides.
  virtual void RegisterInternalExtensionsInExtensionThreadServer(
      extensions::XWalkExtensionServer* server) OVERRIDE;
  virtual void RegisterInternalExtensionsInUIThreadServer(
      extensions::XWalkExtensionServer* server) OVERRIDE;

#if defined(OS_ANDROID)
  RuntimeContext* runtime_context() { return runtime_context_.get(); }

  // XWalkExtensionAndroid needs to register its extensions on
  // XWalkBrowserMainParts so they get correctly registered on-demand
  // by XWalkExtensionService each time a in_process Server is created.
  void RegisterExtension(scoped_ptr<extensions::XWalkExtension> extension);
  void UnregisterExtension(scoped_ptr<extensions::XWalkExtension> extension);
#endif
  extensions::XWalkExtensionService* extension_service() {
    return extension_service_.get();
  }

 protected:
  void RegisterExternalExtensions();

  scoped_ptr<RuntimeContext> runtime_context_;

  // An application wide instance to manage all Runtime instances.
  scoped_ptr<RuntimeRegistry> runtime_registry_;

  scoped_ptr<extensions::XWalkExtensionService> extension_service_;

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
