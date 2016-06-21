// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/devtools/remote_debugging_server.h"

#include <map>
#include <vector>

#include "base/threading/thread_task_runner_handle.h"
#include "components/devtools_http_handler/devtools_http_handler.h"
#include "components/devtools_http_handler/devtools_http_handler_delegate.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_frontend_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "grit/xwalk_resources.h"
#include "net/base/net_errors.h"
#include "net/socket/tcp_server_socket.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/snapshot/snapshot.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/common/xwalk_content_client.h"

using content::DevToolsAgentHost;
using content::RenderViewHost;
using content::RenderWidgetHostView;
using content::WebContents;

namespace xwalk {

class TCPServerSocketFactory
    : public devtools_http_handler::DevToolsHttpHandler::ServerSocketFactory {
 public:
  TCPServerSocketFactory(
    const std::string& address, int port, int backlog)
    : address_(address)
    , backlog_(backlog)
    , port_(port) {
  }

 private:
  // devtools_http_handler::DevToolsHttpHandler::ServerSocketFactory.
  std::unique_ptr<net::ServerSocket> CreateForHttpServer() override {
    std::unique_ptr<net::ServerSocket> socket(
        new net::TCPServerSocket(NULL, net::NetLog::Source()));
    if (socket->ListenWithAddressAndPort(address_, port_, backlog_) != net::OK)
      return std::unique_ptr<net::ServerSocket>();
    return socket;
  }
  std::string address_;
  int backlog_;
  int port_;
  DISALLOW_COPY_AND_ASSIGN(TCPServerSocketFactory);
};

class XWalkDevToolsHttpHandlerDelegate :
    public devtools_http_handler::DevToolsHttpHandlerDelegate {
 public:
  XWalkDevToolsHttpHandlerDelegate();
  ~XWalkDevToolsHttpHandlerDelegate() override;

  // DevToolsHttpHandlerDelegate implementation.
  std::string GetDiscoveryPageHTML() override;
  std::string GetFrontendResource(const std::string& path) override;
  std::string GetPageThumbnailData(const GURL& url) override;
  content::DevToolsExternalAgentProxyDelegate*
      HandleWebSocketConnection(const std::string& path) override;
  void ProcessAndSaveThumbnail(const GURL& url,
                               scoped_refptr<base::RefCountedBytes> png);

 private:
  using ThumbnailMap = std::map<GURL, std::string>;
  ThumbnailMap thumbnail_map_;

  base::WeakPtrFactory<XWalkDevToolsHttpHandlerDelegate> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(XWalkDevToolsHttpHandlerDelegate);
};

XWalkDevToolsHttpHandlerDelegate::XWalkDevToolsHttpHandlerDelegate()
    : weak_factory_(this) {
}

XWalkDevToolsHttpHandlerDelegate::~XWalkDevToolsHttpHandlerDelegate() {
}

std::string XWalkDevToolsHttpHandlerDelegate::GetDiscoveryPageHTML() {
  return ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_DEVTOOLS_FRONTEND_PAGE_HTML).as_string();
}

std::string XWalkDevToolsHttpHandlerDelegate::GetFrontendResource(
    const std::string& path) {
  return content::DevToolsFrontendHost::GetFrontendResource(path).as_string();
}

std::string XWalkDevToolsHttpHandlerDelegate::GetPageThumbnailData(
    const GURL& url) {
  if (thumbnail_map_.find(url) != thumbnail_map_.end())
    return thumbnail_map_[url];
  // TODO(YangangHan): Support real time thumbnail.
  DevToolsAgentHost::List agents =
      content::DevToolsAgentHost::GetOrCreateAll();
  for (auto& it : agents) {
    content::WebContents* web_contents = it.get()->GetWebContents();
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
        base::Bind(&XWalkDevToolsHttpHandlerDelegate::ProcessAndSaveThumbnail,
                   weak_factory_.GetWeakPtr(),
                   url));
        break;
    }
  }
  return std::string();
}

content::DevToolsExternalAgentProxyDelegate*
XWalkDevToolsHttpHandlerDelegate::HandleWebSocketConnection(
    const std::string& path) {
  return nullptr;
}

void XWalkDevToolsHttpHandlerDelegate::ProcessAndSaveThumbnail(
    const GURL& url,
    scoped_refptr<base::RefCountedBytes> png) {
  if (!png.get())
    return;
  const std::vector<unsigned char>& png_data = png->data();
  std::string png_string_data(reinterpret_cast<const char*>(&png_data[0]),
                              png_data.size());
  thumbnail_map_[url] = png_string_data;
}

RemoteDebuggingServer::RemoteDebuggingServer(
    XWalkBrowserContext* browser_context,
    const std::string& ip,
    int port,
    const std::string& frontend_url) {
  base::FilePath output_dir;
  std::unique_ptr<devtools_http_handler::DevToolsHttpHandler::ServerSocketFactory>
      factory(new TCPServerSocketFactory(ip, port, 1));
  devtools_http_handler_.reset(new devtools_http_handler::DevToolsHttpHandler(
          std::move(factory),
          frontend_url,
          new XWalkDevToolsHttpHandlerDelegate(),
          output_dir,
          output_dir,
          std::string(),
          xwalk::GetUserAgent()));
  port_ = port;
}

RemoteDebuggingServer::~RemoteDebuggingServer() {
}

}  // namespace xwalk
