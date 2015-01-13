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

  std::string GetId() const override { return agent_host_->GetId(); }
  std::string GetType() const override {
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
  std::string GetTitle() const override {
    return agent_host_->GetTitle();
  }
  std::string GetDescription() const override { return std::string(); }
  GURL GetURL() const override { return  agent_host_->GetURL(); }
  GURL GetFaviconURL() const override { return favicon_url_; }
  base::TimeTicks GetLastActivityTime() const override {
    return last_activity_time_;
  }
  std::string GetParentId() const override { return std::string(); }
  bool IsAttached() const override {
    return agent_host_->IsAttached();
  }
  scoped_refptr<DevToolsAgentHost> GetAgentHost() const override {
    return agent_host_;
  }
  bool Activate() const override;
  bool Close() const override;

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
#if defined(OS_ANDROID)
  // TODO(YangangHan): Add a new base parent class of WebContents
  // for both Tizen and Android, so we can remove the current macro
  // in the future.
  return GURL();
#endif
  xwalk::Runtime* runtime =
      static_cast<xwalk::Runtime*>(web_contents->GetDelegate());
  if (!runtime || runtime->app_icon().IsEmpty())
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

namespace {
Runtime* CreateWithDefaultWindow(
    XWalkBrowserContext* browser_context, const GURL& url,
    Runtime::Observer* observer) {
  Runtime* runtime = Runtime::Create(browser_context);
  runtime->set_observer(observer);
  runtime->LoadURL(url);
#if !defined(OS_ANDROID)
  runtime->set_ui_delegate(DefaultRuntimeUIDelegate::Create(runtime));
  runtime->Show();
#endif
  return runtime;
}
}  // namespace

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
  if (!png.get())
    return;
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
  Runtime* runtime = CreateWithDefaultWindow(
      browser_context_, url, this);
  runtime->set_remote_debugging_enabled(true);
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

void XWalkDevToolsDelegate::OnNewRuntimeAdded(Runtime* runtime) {
  runtime->set_observer(this);
  runtime->set_ui_delegate(DefaultRuntimeUIDelegate::Create(runtime));
  runtime->Show();
}

void XWalkDevToolsDelegate::OnRuntimeClosed(Runtime* runtime) {
  delete runtime;
}

}  // namespace xwalk
