// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/devtools/xwalk_devtools_delegate.h"

#include <string>

#include "base/base64.h"
#include "base/memory/ref_counted_memory.h"
#include "base/strings/utf_string_conversions.h"
#include "base/thread_task_runner_handle.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/devtools_target.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/url_constants.h"
#include "grit/xwalk_resources.h"
#include "net/socket/tcp_listen_socket.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/snapshot/snapshot.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"

#if !defined(OS_ANDROID)
#include "xwalk/runtime/browser/runtime_ui_delegate.h"
#endif

using content::DevToolsAgentHost;
using content::RenderViewHost;
using content::RenderWidgetHostView;
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
#if !defined(OS_ANDROID)
  // Convert icon image to "data:" url.
  xwalk::XWalkContent* content =
      static_cast<xwalk::XWalkContent*>(web_contents->GetDelegate());
  if (!content || content->app_icon().IsEmpty())
    return GURL();
  scoped_refptr<base::RefCountedMemory> icon_bytes =
      content->app_icon().Copy1xPNGBytes();
  std::string str_url;
  str_url.append(reinterpret_cast<const char*>(icon_bytes->front()),
                 icon_bytes->size());
  base::Base64Encode(str_url, &str_url);
  str_url.insert(0, "data:image/png;base64,");
  return GURL(str_url);
#else
  return GURL();
#endif
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

void XWalkDevToolsDelegate::ProcessAndSaveThumbnail(
    const GURL& url,
    scoped_refptr<base::RefCountedBytes> png) {
  const std::vector<unsigned char>& png_data = png->data();
  std::string png_string_data(reinterpret_cast<const char*>(&png_data[0]),
                              png_data.size());
  thumbnail_map_[url] = png_string_data;
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

XWalkDevToolsDelegate::XWalkDevToolsDelegate(XWalkBrowserContext* context)
    : browser_context_(context),
      weak_factory_(this) {
}

XWalkDevToolsDelegate::~XWalkDevToolsDelegate() {
}

base::DictionaryValue* XWalkDevToolsDelegate::HandleCommand(
    content::DevToolsAgentHost* agent_host,
    base::DictionaryValue* command_dict) {
  return NULL;
}

std::string XWalkDevToolsDelegate::GetPageThumbnailData(const GURL& url) {
  if (thumbnail_map_.find(url) != thumbnail_map_.end())
    return thumbnail_map_[url];
  // TODO(YangangHan): Support real time thumbnail.
  content::DevToolsAgentHost::List agents =
      content::DevToolsAgentHost::GetOrCreateAll();
  for (auto& it : agents) {
    WebContents* web_contents = it.get()->GetWebContents();
    if (web_contents && web_contents->GetURL() == url) {
      RenderWidgetHostView* render_widget_host_view =
          web_contents->GetRenderWidgetHostView();
      if (!render_widget_host_view)
        continue;
      gfx::Rect snapshot_bounds(
        render_widget_host_view->GetViewBounds().size());
      ui::GrabViewSnapshotAsync(
        render_widget_host_view->GetNativeView(),
        snapshot_bounds,
        base::ThreadTaskRunnerHandle::Get(),
        base::Bind(&XWalkDevToolsDelegate::ProcessAndSaveThumbnail,
                   weak_factory_.GetWeakPtr(),
                   url));
        break;
    }
  }
  return std::string();
}

scoped_ptr<content::DevToolsTarget>
XWalkDevToolsDelegate::CreateNewTarget(const GURL& url) {
  using content::WebContents;
#if !defined(OS_ANDROID)
  XWalkContent* content = XWalkContent::Create(browser_context_);
  content->set_observer(this);
  content->set_ui_delegate(DefaultRuntimeUIDelegate::Create(content));
  content->LoadURL(GURL(url::kAboutBlankURL));
  content->Show();
  WebContents* web_contents = content->web_contents();
#else
  // FIXME : this is just repeating of the previously existed code path.
  // However the implementation for Android MUST be reconsidered.
  WebContents::CreateParams params(browser_context_);
  params.routing_id = MSG_ROUTING_NONE;
  WebContents* web_contents = WebContents::Create(params);
  content::NavigationController::LoadURLParams load_params(
      GURL(url::kAboutBlankURL));
  load_params.transition_type = ui::PageTransitionFromInt(
      ui::PAGE_TRANSITION_TYPED | ui::PAGE_TRANSITION_FROM_ADDRESS_BAR);
  web_contents->GetController().LoadURLWithParams(load_params);
  web_contents->Focus();
#endif
  return scoped_ptr<content::DevToolsTarget>(
      new Target(DevToolsAgentHost::GetOrCreateFor(web_contents)));
}

void XWalkDevToolsDelegate::EnumerateTargets(TargetCallback callback) {
  TargetList targets;
  content::DevToolsAgentHost::List agents =
      content::DevToolsAgentHost::GetOrCreateAll();
  for (content::DevToolsAgentHost::List::iterator it = agents.begin();
       it != agents.end(); ++it) {
#if !defined(OS_ANDROID)
    XWalkContent* content =
        static_cast<XWalkContent*>((*it)->GetWebContents()->GetDelegate());
    if (content && content->remote_debugging_enabled())
#endif
      targets.push_back(new Target(*it));
  }
  callback.Run(targets);
}
#if !defined(OS_ANDROID)
void XWalkDevToolsDelegate::OnContentCreated(XWalkContent* content) {
  content->set_observer(this);
  content->set_ui_delegate(DefaultRuntimeUIDelegate::Create(content));
  content->Show();
}

void XWalkDevToolsDelegate::OnContentClosed(XWalkContent* content) {
  delete content;
}
#endif

}  // namespace xwalk
