// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_DEVTOOLS_XWALK_DEVTOOLS_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_DEVTOOLS_XWALK_DEVTOOLS_DELEGATE_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/browser/devtools_http_handler_delegate.h"
#include "content/public/browser/devtools_manager_delegate.h"
#include "url/gurl.h"

namespace content {
class DevToolsHttpHandler;
}

namespace xwalk {

class RuntimeContext;

class XWalkDevToolsHttpHandlerDelegate :
    public content::DevToolsHttpHandlerDelegate {
 public:
  XWalkDevToolsHttpHandlerDelegate();
  virtual ~XWalkDevToolsHttpHandlerDelegate();

  // DevToolsHttpHandlerDelegate implementation.
  virtual std::string GetDiscoveryPageHTML() OVERRIDE;
  virtual bool BundlesFrontendResources() OVERRIDE;
  virtual base::FilePath GetDebugFrontendDir() OVERRIDE;
  virtual scoped_ptr<net::StreamListenSocket> CreateSocketForTethering(
      net::StreamListenSocket::Delegate* delegate,
      std::string* name) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkDevToolsHttpHandlerDelegate);
};

class XWalkDevToolsDelegate : public content::DevToolsManagerDelegate {
 public:
  explicit XWalkDevToolsDelegate(RuntimeContext* runtime_context);
  virtual ~XWalkDevToolsDelegate();

  virtual void Inspect(
      content::BrowserContext* browser_context,
      content::DevToolsAgentHost* agent_host) OVERRIDE {}
  virtual void DevToolsAgentStateChanged(
      content::DevToolsAgentHost* agent_host,
      bool attached) OVERRIDE {}
  virtual base::DictionaryValue* HandleCommand(
      content::DevToolsAgentHost* agent_host,
      base::DictionaryValue* command_dict) OVERRIDE;
  virtual scoped_ptr<content::DevToolsTarget> CreateNewTarget(
      const GURL& url) OVERRIDE;
  virtual void EnumerateTargets(TargetCallback callback) OVERRIDE;
  virtual std::string GetPageThumbnailData(const GURL& url) OVERRIDE;

 private:
  RuntimeContext* runtime_context_;
  DISALLOW_COPY_AND_ASSIGN(XWalkDevToolsDelegate);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_DEVTOOLS_XWALK_DEVTOOLS_DELEGATE_H_
