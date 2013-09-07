// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_H_
#define XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/common/main_function_params.h"
#include "googleurl/src/gurl.h"

namespace xwalk {

namespace extensions {
class XWalkExtensionService;
}

class RuntimeContext;
class RuntimeRegistry;
class RemoteDebuggingServer;

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

#if defined(OS_ANDROID)
  void SetRuntimeContext(RuntimeContext* context);
  RuntimeContext* runtime_context() { return runtime_context_; }
#else
  RuntimeContext* runtime_context() { return runtime_context_.get(); }
#endif
  extensions::XWalkExtensionService* extension_service() {
    return extension_service_.get();
  }

 private:
  void RegisterExternalExtensions();
  void RegisterInternalExtensions();
#if defined(OS_MACOSX)
  void PreMainMessageLoopStartMac();
#elif defined(USE_AURA)
  void PreMainMessageLoopStartAura();
  void PostMainMessageLoopRunAura();
#endif
/*
#if defined(OS_TIZEN_MOBILE)
  std::string& GetGLImplementation();
#endif
*/
#if defined(OS_ANDROID)
  RuntimeContext* runtime_context_;
#else
  scoped_ptr<RuntimeContext> runtime_context_;
#endif

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

  DISALLOW_COPY_AND_ASSIGN(XWalkBrowserMainParts);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_H_
