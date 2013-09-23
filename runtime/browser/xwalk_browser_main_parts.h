// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_H_
#define XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_H_

#include <string>
#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/common/main_function_params.h"
#include "googleurl/src/gurl.h"
#include "xwalk/runtime/browser/xwalk_process_singleton.h"

namespace xwalk {

namespace extensions {
class XWalkExtensionService;
}

class RuntimeContext;
class RuntimeRegistry;
class RemoteDebuggingServer;

void SetXWalkCommandLineFlags();

class XWalkBrowserMainParts : public content::BrowserMainParts {
 public:
  static XWalkBrowserMainParts* GetInstance();

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
  virtual void PostDestroyThreads() OVERRIDE;

#if defined(OS_ANDROID)
  void SetRuntimeContext(RuntimeContext* context);
  RuntimeContext* runtime_context() { return runtime_context_; }
#else
  RuntimeContext* runtime_context() { return runtime_context_.get(); }
#endif
  extensions::XWalkExtensionService* extension_service() {
    return extension_service_.get();
  }
  // Handles application launch/install/uninstall/... as well as loading a web
  // page directly from an URL.
  void ProcessCommandLine(const CommandLine* command_line);

 private:
  void RegisterExternalExtensions();
  void RegisterInternalExtensions();
#if defined(OS_MACOSX)
  void PreMainMessageLoopStartMac();
#elif defined(USE_AURA)
  void PreMainMessageLoopStartAura();
  void PostMainMessageLoopRunAura();
#endif

#if defined(OS_TIZEN_MOBILE)
  bool HandlePackageInfo(const std::string& id, const std::string& option);
#endif

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

#if defined(OS_LINUX)
  // Allows different browser processes to communicate with each other.
  ProcessSingleton::NotifyResult notify_result_;
  scoped_ptr<ProcessSingleton> process_singleton_;
#endif

  DISALLOW_COPY_AND_ASSIGN(XWalkBrowserMainParts);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_BROWSER_MAIN_PARTS_H_
