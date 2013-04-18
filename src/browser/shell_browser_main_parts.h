// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_BROWSER_SHELL_BROWSER_MAIN_PARTS_H_
#define CAMEO_SRC_BROWSER_SHELL_BROWSER_MAIN_PARTS_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_main_parts.h"

namespace base {
class Thread;
}

namespace content {
struct MainFunctionParams;
}

namespace cameo {

class ShellBrowserContext;
class ShellDevToolsDelegate;

class ShellBrowserMainParts : public content::BrowserMainParts {
 public:
  explicit ShellBrowserMainParts(
      const content::MainFunctionParams& parameters);
  virtual ~ShellBrowserMainParts();

  // BrowserMainParts overrides.
  virtual void PreEarlyInitialization() OVERRIDE;
  virtual void PreMainMessageLoopStart() OVERRIDE;
  virtual void PostMainMessageLoopStart() OVERRIDE;
  virtual void PreMainMessageLoopRun() OVERRIDE;
  virtual bool MainMessageLoopRun(int* result_code) OVERRIDE;
  virtual void PostMainMessageLoopRun() OVERRIDE;

  ShellDevToolsDelegate* devtools_delegate() { return devtools_delegate_; }

  ShellBrowserContext* browser_context() { return browser_context_.get(); }
  ShellBrowserContext* off_the_record_browser_context() {
    return off_the_record_browser_context_.get();
  }

 private:
  scoped_ptr<ShellBrowserContext> browser_context_;
  scoped_ptr<ShellBrowserContext> off_the_record_browser_context_;

  // For running content_browsertests.
  const content::MainFunctionParams& parameters_;
  bool run_message_loop_;

  ShellDevToolsDelegate* devtools_delegate_;

  DISALLOW_COPY_AND_ASSIGN(ShellBrowserMainParts);
};

}  // namespace cameo

#endif  // CAMEO_SRC_BROWSER_SHELL_BROWSER_MAIN_PARTS_H_
