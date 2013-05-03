// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_SHELL_DEVTOOLS_DELEGATE_H_
#define CONTENT_SHELL_SHELL_DEVTOOLS_DELEGATE_H_

#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/browser/devtools_http_handler_delegate.h"

namespace content {

class BrowserContext;
class DevToolsHttpHandler;

class ShellDevToolsDelegate : public DevToolsHttpHandlerDelegate {
 public:
  ShellDevToolsDelegate(BrowserContext* browser_context, int port);
  virtual ~ShellDevToolsDelegate();

  // Stops http server.
  void Stop();

  // DevToolsHttpProtocolHandler::Delegate overrides.
  virtual std::string GetDiscoveryPageHTML() OVERRIDE;
  virtual bool BundlesFrontendResources() OVERRIDE;
  virtual base::FilePath GetDebugFrontendDir() OVERRIDE;
  virtual std::string GetPageThumbnailData(const GURL& url) OVERRIDE;
  virtual RenderViewHost* CreateNewTarget() OVERRIDE;
  virtual TargetType GetTargetType(RenderViewHost*) OVERRIDE;
  virtual std::string GetViewDescription(content::RenderViewHost*) OVERRIDE;

  DevToolsHttpHandler* devtools_http_handler() {
    return devtools_http_handler_;
  }

 private:
  BrowserContext* browser_context_;
  DevToolsHttpHandler* devtools_http_handler_;

  DISALLOW_COPY_AND_ASSIGN(ShellDevToolsDelegate);
};

}  // namespace content

#endif  // CONTENT_SHELL_SHELL_DEVTOOLS_DELEGATE_H_
