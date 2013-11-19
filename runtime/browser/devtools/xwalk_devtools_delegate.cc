// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/devtools/xwalk_devtools_delegate.h"

#include <string>

#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/devtools_target.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/url_constants.h"
#include "grit/xwalk_resources.h"
#include "net/socket/tcp_listen_socket.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/runtime/browser/runtime.h"

using content::DevToolsAgentHost;
using content::RenderViewHost;
using content::WebContents;

namespace {

const char kTargetTypePage[] = "page";

class Target : public content::DevToolsTarget {
 public:
  explicit Target(WebContents* web_contents);

  virtual std::string GetId() const OVERRIDE { return id_; }
  virtual std::string GetType() const OVERRIDE { return kTargetTypePage; }
  virtual std::string GetTitle() const OVERRIDE { return title_; }
  virtual std::string GetDescription() const OVERRIDE { return std::string(); }
  virtual GURL GetUrl() const OVERRIDE { return url_; }
  virtual GURL GetFaviconUrl() const OVERRIDE { return favicon_url_; }
  virtual base::TimeTicks GetLastActivityTime() const OVERRIDE {
    return last_activity_time_;
  }
  virtual bool IsAttached() const OVERRIDE {
    return agent_host_->IsAttached();
  }
  virtual scoped_refptr<DevToolsAgentHost> GetAgentHost() const OVERRIDE {
    return agent_host_;
  }
  virtual bool Activate() const OVERRIDE;
  virtual bool Close() const OVERRIDE;

 private:
  scoped_refptr<DevToolsAgentHost> agent_host_;
  std::string id_;
  std::string title_;
  GURL url_;
  GURL favicon_url_;
  base::TimeTicks last_activity_time_;
};

Target::Target(WebContents* web_contents) {
  agent_host_ =
      DevToolsAgentHost::GetOrCreateFor(web_contents->GetRenderViewHost());
  id_ = agent_host_->GetId();
  title_ = UTF16ToUTF8(web_contents->GetTitle());
  url_ = web_contents->GetURL();
  content::NavigationController& controller = web_contents->GetController();
  content::NavigationEntry* entry = controller.GetActiveEntry();
  if (entry != NULL && entry->GetURL().is_valid())
    favicon_url_ = entry->GetFavicon().url;
  last_activity_time_ = web_contents->GetLastSelectedTime();
}

bool Target::Activate() const {
  RenderViewHost* rvh = agent_host_->GetRenderViewHost();
  if (!rvh)
    return false;
  WebContents* web_contents = WebContents::FromRenderViewHost(rvh);
  if (!web_contents)
    return false;
  web_contents->GetDelegate()->ActivateContents(web_contents);
  return true;
}

bool Target::Close() const {
  RenderViewHost* rvh = agent_host_->GetRenderViewHost();
  if (!rvh)
    return false;
  rvh->ClosePage();
  return true;
}

}  // namespace

namespace xwalk {

XWalkDevToolsDelegate::XWalkDevToolsDelegate(RuntimeContext* runtime_context)
    : runtime_context_(runtime_context) {
}

XWalkDevToolsDelegate::~XWalkDevToolsDelegate() {
}

std::string XWalkDevToolsDelegate::GetDiscoveryPageHTML() {
  return ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_DEVTOOLS_FRONTEND_PAGE_HTML).as_string();
}

bool XWalkDevToolsDelegate::BundlesFrontendResources() {
  return true;
}

base::FilePath XWalkDevToolsDelegate::GetDebugFrontendDir() {
  return base::FilePath();
}

std::string XWalkDevToolsDelegate::GetPageThumbnailData(const GURL& url) {
  return std::string();
}

scoped_ptr<content::DevToolsTarget>
XWalkDevToolsDelegate::CreateNewTarget(const GURL& url) {
  Runtime* runtime = Runtime::CreateWithDefaultWindow(
      runtime_context_, GURL(content::kAboutBlankURL));
  return scoped_ptr<content::DevToolsTarget>(
      new Target(runtime->web_contents()));
}

void XWalkDevToolsDelegate::EnumerateTargets(TargetCallback callback) {
  TargetList targets;
  std::vector<RenderViewHost*> rvh_list =
      content::DevToolsAgentHost::GetValidRenderViewHosts();
  for (std::vector<RenderViewHost*>::iterator it = rvh_list.begin();
       it != rvh_list.end(); ++it) {
    WebContents* web_contents = WebContents::FromRenderViewHost(*it);
    if (web_contents)
      targets.push_back(new Target(web_contents));
  }
  callback.Run(targets);
}

scoped_ptr<net::StreamListenSocket>
XWalkDevToolsDelegate::CreateSocketForTethering(
      net::StreamListenSocket::Delegate* delegate,
      std::string* name) {
  return scoped_ptr<net::StreamListenSocket>();
}

}  // namespace xwalk
