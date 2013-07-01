// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_RUNTIME_BROWSER_CAMEO_BROWSER_MAIN_PARTS_H_
#define CAMEO_RUNTIME_BROWSER_CAMEO_BROWSER_MAIN_PARTS_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/common/main_function_params.h"
#include "googleurl/src/gurl.h"

namespace cameo {

namespace extensions {
class CameoExtensionService;
}

class RuntimeContext;
class RuntimeRegistry;
class RemoteDebuggingServer;

class CameoBrowserMainParts : public content::BrowserMainParts {
 public:
  explicit CameoBrowserMainParts(
      const content::MainFunctionParams& parameters);
  virtual ~CameoBrowserMainParts();

  // BrowserMainParts overrides.
  virtual void PreEarlyInitialization() OVERRIDE;
  virtual void PreMainMessageLoopStart() OVERRIDE;
  virtual void PostMainMessageLoopStart() OVERRIDE;
  virtual void PreMainMessageLoopRun() OVERRIDE;
  virtual bool MainMessageLoopRun(int* result_code) OVERRIDE;
  virtual void PostMainMessageLoopRun() OVERRIDE;

  RuntimeContext* runtime_context() { return runtime_context_.get(); }
  extensions::CameoExtensionService* extension_service() {
    return extension_service_.get();
  }

 private:
  void RegisterExternalExtensions();

  scoped_ptr<RuntimeContext> runtime_context_;

  // An application wide instance to manage all Runtime instances.
  scoped_ptr<RuntimeRegistry> runtime_registry_;

  scoped_ptr<extensions::CameoExtensionService> extension_service_;

  // Should be about:blank If no URL is specified in command line arguments.
  GURL startup_url_;

  // The main function parameters passed to BrowserMain.
  const content::MainFunctionParams& parameters_;

  // True if we need to run the default message loop defined in content.
  bool run_default_message_loop_;

  // Remote debugger server.
  scoped_ptr<RemoteDebuggingServer> remote_debugging_server_;

  DISALLOW_COPY_AND_ASSIGN(CameoBrowserMainParts);
};

}  // namespace cameo

#endif  // CAMEO_RUNTIME_BROWSER_CAMEO_BROWSER_MAIN_PARTS_H_
