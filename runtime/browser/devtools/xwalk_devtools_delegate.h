// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_DEVTOOLS_XWALK_DEVTOOLS_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_DEVTOOLS_XWALK_DEVTOOLS_DELEGATE_H_

#include <map>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted_memory.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/devtools_http_handler_delegate.h"
#include "content/public/browser/devtools_manager_delegate.h"
#include "url/gurl.h"
#include "xwalk/runtime/browser/runtime.h"

namespace content {
class DevToolsHttpHandler;
}

namespace xwalk {

class XWalkBrowserContext;

class XWalkDevToolsHttpHandlerDelegate :
    public content::DevToolsHttpHandlerDelegate {
 public:
  XWalkDevToolsHttpHandlerDelegate();
  virtual ~XWalkDevToolsHttpHandlerDelegate();

  // DevToolsHttpHandlerDelegate implementation.
  std::string GetDiscoveryPageHTML() override;
  bool BundlesFrontendResources() override;
  base::FilePath GetDebugFrontendDir() override;
  scoped_ptr<net::StreamListenSocket> CreateSocketForTethering(
      net::StreamListenSocket::Delegate* delegate,
      std::string* name) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkDevToolsHttpHandlerDelegate);
};

class XWalkDevToolsDelegate : public content::DevToolsManagerDelegate,
                              public Runtime::Observer {
 public:
  explicit XWalkDevToolsDelegate(XWalkBrowserContext* browser_context);
  virtual ~XWalkDevToolsDelegate();

  void Inspect(
      content::BrowserContext* browser_context,
      content::DevToolsAgentHost* agent_host) override {}
  void DevToolsAgentStateChanged(
      content::DevToolsAgentHost* agent_host,
      bool attached) override {}
  base::DictionaryValue* HandleCommand(
      content::DevToolsAgentHost* agent_host,
      base::DictionaryValue* command_dict) override;
  scoped_ptr<content::DevToolsTarget> CreateNewTarget(
      const GURL& url) override;
  void EnumerateTargets(TargetCallback callback) override;
  std::string GetPageThumbnailData(const GURL& url) override;
  void ProcessAndSaveThumbnail(const GURL& url,
                               scoped_refptr<base::RefCountedBytes> png);

 private:
  // Runtime::Observer
  virtual void OnNewRuntimeAdded(Runtime* runtime) override;
  virtual void OnRuntimeClosed(Runtime* runtime) override;

  using ThumbnailMap = std::map<GURL, std::string>;
  ThumbnailMap thumbnail_map_;
  XWalkBrowserContext* browser_context_;
  base::WeakPtrFactory<XWalkDevToolsDelegate> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(XWalkDevToolsDelegate);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_DEVTOOLS_XWALK_DEVTOOLS_DELEGATE_H_
