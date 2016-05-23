// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_DEVTOOLS_XWALK_DEVTOOLS_FRONTEND_H_
#define XWALK_RUNTIME_BROWSER_DEVTOOLS_XWALK_DEVTOOLS_FRONTEND_H_

#include <map>
#include <memory>
#include <string>

#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_frontend_host.h"
#include "content/public/browser/web_contents_observer.h"
#include "net/url_request/url_fetcher_delegate.h"
#include "xwalk/runtime/browser/runtime.h"

namespace base {
class Value;
}

namespace content {
class RenderViewHost;
class WebContents;
}

namespace xwalk {

class NativeAppWindowDesktop;

class XWalkDevToolsFrontend : public content::WebContentsObserver,
                              public content::DevToolsAgentHostClient,
                              public net::URLFetcherDelegate,
                              public Runtime::Observer {
 public:
  static XWalkDevToolsFrontend* Show(content::WebContents* inspected_contents);

  void Activate();
  void Focus();
  void InspectElementAt(int x, int y);
  void Close();

  void DisconnectFromTarget();

  NativeAppWindowDesktop* frontend_shell() const { return runtime_window_; }

  void CallClientFunction(const std::string& function_name,
                          const base::Value* arg1,
                          const base::Value* arg2,
                          const base::Value* arg3);

 protected:
  XWalkDevToolsFrontend(NativeAppWindowDesktop* runtime_window,
                        content::WebContents* inspector_contents,
                        content::WebContents* inspected_contents);
  ~XWalkDevToolsFrontend() override;

  void OnRuntimeClosed(Runtime* runtime) override;
  void OnNewRuntimeAdded(Runtime* runtime) override {};
  void OnApplicationExitRequested(Runtime* runtime) override {}

  // content::DevToolsAgentHostClient implementation.
  void AgentHostClosed(
      content::DevToolsAgentHost* agent_host, bool replaced) override;
  void DispatchProtocolMessage(content::DevToolsAgentHost* agent_host,
                               const std::string& message) override;

 private:
  // WebContentsObserver overrides
  void RenderViewCreated(content::RenderViewHost* render_view_host) override;
  void DocumentAvailableInMainFrame() override;
  void WebContentsDestroyed() override;

  // content::DevToolsFrontendHost::Delegate implementation.
  void HandleMessageFromDevToolsFrontend(const std::string& message);

  // net::URLFetcherDelegate overrides.
  void OnURLFetchComplete(const net::URLFetcher* source) override;

  void SendMessageAck(int request_id,
                      const base::Value* arg1);

  NativeAppWindowDesktop* runtime_window_;
  content::WebContents* inspected_contents_;
  scoped_refptr<content::DevToolsAgentHost> agent_host_;
  std::unique_ptr<content::DevToolsFrontendHost> frontend_host_;
  using PendingRequestsMap = std::map<const net::URLFetcher*, int>;
  PendingRequestsMap pending_requests_;
  base::DictionaryValue preferences_;
  base::WeakPtrFactory<XWalkDevToolsFrontend> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(XWalkDevToolsFrontend);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_DEVTOOLS_XWALK_DEVTOOLS_FRONTEND_H_
