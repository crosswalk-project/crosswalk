// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/devtools/xwalk_devtools_delegate.h"

#include <string>

#include "base/base64.h"
#include "base/memory/ref_counted_memory.h"
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
const char kTargetTypeServiceWorker[] = "service_worker";
const char kTargetTypeOther[] = "other";

class Target : public content::DevToolsTarget {
 public:
  explicit Target(scoped_refptr<content::DevToolsAgentHost> agent_host);

  virtual std::string GetId() const OVERRIDE { return agent_host_->GetId(); }
  virtual std::string GetType() const OVERRIDE {
      switch (agent_host_->GetType()) {
        case content::DevToolsAgentHost::TYPE_WEB_CONTENTS:
           return kTargetTypePage;
         case content::DevToolsAgentHost::TYPE_SERVICE_WORKER:
           return kTargetTypeServiceWorker;
         default:
           break;
       }
       return kTargetTypeOther;
     }
  virtual std::string GetTitle() const OVERRIDE {
    return agent_host_->GetTitle();
  }
  virtual std::string GetDescription() const OVERRIDE { return std::string(); }
  virtual GURL GetURL() const OVERRIDE { return  agent_host_->GetURL(); }
  virtual GURL GetFaviconURL() const OVERRIDE { return favicon_url_; }
  virtual base::TimeTicks GetLastActivityTime() const OVERRIDE {
    return last_activity_time_;
  }
  virtual std::string GetParentId() const OVERRIDE { return std::string(); }
  virtual bool IsAttached() const OVERRIDE {
    return agent_host_->IsAttached();
  }
  virtual scoped_refptr<DevToolsAgentHost> GetAgentHost() const OVERRIDE {
    return agent_host_;
  }
  virtual bool Activate() const OVERRIDE;
  virtual bool Close() const OVERRIDE;

 private:
  GURL GetFaviconDataURL(WebContents* web_contents) const;

  scoped_refptr<DevToolsAgentHost> agent_host_;
  std::string id_;
  std::string title_;
  GURL favicon_url_;
  base::TimeTicks last_activity_time_;
};

Target::Target(scoped_refptr<content::DevToolsAgentHost> agent_host)
    : agent_host_(agent_host) {
  if (content::WebContents* web_contents = agent_host_->GetWebContents()) {
    content::NavigationController& controller = web_contents->GetController();
    content::NavigationEntry* entry = controller.GetActiveEntry();
    if (entry != NULL && entry->GetURL().is_valid())
      favicon_url_ = entry->GetFavicon().url;
    if (favicon_url_.is_empty())
      favicon_url_ = GetFaviconDataURL(web_contents);
    last_activity_time_ = web_contents->GetLastActiveTime();
  }
}

GURL Target::GetFaviconDataURL(WebContents* web_contents) const {
  // Convert icon image to "data:" url.
  xwalk::Runtime* runtime =
      static_cast<xwalk::Runtime*>(web_contents->GetDelegate());
  if (!runtime)
    return GURL();
  scoped_refptr<base::RefCountedMemory> icon_bytes =
      runtime->app_icon().Copy1xPNGBytes();
  std::string str_url;
  str_url.append(reinterpret_cast<const char*>(icon_bytes->front()),
                 icon_bytes->size());
  base::Base64Encode(str_url, &str_url);
  str_url.insert(0, "data:image/png;base64,");
  return GURL(str_url);
}

bool Target::Activate() const {
  return agent_host_->Activate();
}

bool Target::Close() const {
  return agent_host_->Close();
}

}  // namespace

namespace xwalk {

XWalkDevToolsHttpHandlerDelegate::XWalkDevToolsHttpHandlerDelegate() {
}

XWalkDevToolsHttpHandlerDelegate::~XWalkDevToolsHttpHandlerDelegate() {
}

std::string XWalkDevToolsHttpHandlerDelegate::GetDiscoveryPageHTML() {
  return ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_DEVTOOLS_FRONTEND_PAGE_HTML).as_string();
}

bool XWalkDevToolsHttpHandlerDelegate::BundlesFrontendResources() {
  return true;
}

base::FilePath XWalkDevToolsHttpHandlerDelegate::GetDebugFrontendDir() {
  return base::FilePath();
}

scoped_ptr<net::StreamListenSocket>
XWalkDevToolsHttpHandlerDelegate::CreateSocketForTethering(
    net::StreamListenSocket::Delegate* delegate,
    std::string* name) {
  return scoped_ptr<net::StreamListenSocket>();
}

XWalkDevToolsDelegate::XWalkDevToolsDelegate(RuntimeContext* runtime_context)
    : runtime_context_(runtime_context) {
}

XWalkDevToolsDelegate::~XWalkDevToolsDelegate() {
}

base::DictionaryValue* XWalkDevToolsDelegate::HandleCommand(
    content::DevToolsAgentHost* agent_host,
    base::DictionaryValue* command_dict) {
  return NULL;
}

std::string XWalkDevToolsDelegate::GetPageThumbnailData(const GURL& url) {
  return std::string();
}

scoped_ptr<content::DevToolsTarget>
XWalkDevToolsDelegate::CreateNewTarget(const GURL& url) {
  Runtime* runtime = Runtime::CreateWithDefaultWindow(
      runtime_context_, GURL(url::kAboutBlankURL));
  return scoped_ptr<content::DevToolsTarget>(
      new Target(DevToolsAgentHost::GetOrCreateFor(runtime->web_contents())));
}

void XWalkDevToolsDelegate::EnumerateTargets(TargetCallback callback) {
  TargetList targets;
  content::DevToolsAgentHost::List agents =
      content::DevToolsAgentHost::GetOrCreateAll();
  for (content::DevToolsAgentHost::List::iterator it = agents.begin();
       it != agents.end(); ++it) {
#if !defined(OS_ANDROID)
    Runtime* runtime =
        static_cast<Runtime*>((*it)->GetWebContents()->GetDelegate());
    if (runtime && runtime->remote_debugging_enabled())
#endif
      targets.push_back(new Target(*it));
  }
  callback.Run(targets);
}

}  // namespace xwalk
