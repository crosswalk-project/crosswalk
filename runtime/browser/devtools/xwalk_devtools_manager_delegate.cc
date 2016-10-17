// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/devtools/xwalk_devtools_manager_delegate.h"

#include <string>
#include <vector>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "components/devtools_discovery/basic_target_descriptor.h"
#include "components/devtools_discovery/devtools_discovery_manager.h"
#include "components/devtools_http_handler/devtools_http_handler.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_frontend_host.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/user_agent.h"
#include "content/shell/browser/shell.h"
#include "content/shell/common/shell_content_client.h"
#include "grit/xwalk_resources.h"
#include "net/base/net_errors.h"
#include "net/socket/tcp_server_socket.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/runtime/common/xwalk_content_client.h"
#include "xwalk/runtime/browser/runtime.h"

using devtools_http_handler::DevToolsHttpHandler;

namespace xwalk {

namespace {

const int kBackLog = 10;
const char kLocalHost[] = "127.0.0.1";

uint16_t GetInspectorPort() {
  const base::CommandLine& command_line =
    *base::CommandLine::ForCurrentProcess();
  // See if the user specified a port on the command line (useful for
  // automation). If not, use an ephemeral port by specifying 0.
  uint16_t port = 0;
  if (command_line.HasSwitch(switches::kRemoteDebuggingPort)) {
    int temp_port;
    std::string port_str =
      command_line.GetSwitchValueASCII(switches::kRemoteDebuggingPort);
    if (base::StringToInt(port_str, &temp_port) &&
      temp_port > 0 && temp_port < 65535) {
      port = static_cast<uint16_t>(temp_port);
    } else {
      DLOG(WARNING) << "Invalid http debugger port number " << temp_port;
    }
  }
  return port;
}

class TCPServerSocketFactory
    : public DevToolsHttpHandler::ServerSocketFactory {
 public:
  TCPServerSocketFactory(const std::string& address, uint16_t port)
      : address_(address), port_(port) {
  }

 private:
  // DevToolsHttpHandler::ServerSocketFactory.
  std::unique_ptr<net::ServerSocket> CreateForHttpServer() override {
    std::unique_ptr<net::ServerSocket> socket(
        new net::TCPServerSocket(nullptr, net::NetLog::Source()));
    if (socket->ListenWithAddressAndPort(address_, port_, kBackLog) != net::OK)
      return std::unique_ptr<net::ServerSocket>();

    return socket;
  }

  std::string address_;
  uint16_t port_;

  DISALLOW_COPY_AND_ASSIGN(TCPServerSocketFactory);
};

std::unique_ptr<DevToolsHttpHandler::ServerSocketFactory>
CreateSocketFactory(uint16_t port) {
  return std::unique_ptr<DevToolsHttpHandler::ServerSocketFactory>(
      new TCPServerSocketFactory(kLocalHost, port));
}

std::unique_ptr<devtools_discovery::DevToolsTargetDescriptor>
CreateNewShellTarget(XWalkBrowserContext* browser_context, const GURL& url) {
  Runtime* runtime = Runtime::Create(browser_context);
  return base::WrapUnique(new devtools_discovery::BasicTargetDescriptor(
      content::DevToolsAgentHost::GetOrCreateFor(runtime->web_contents())));
}

class XWalkDevToolsDelegate :
    public devtools_http_handler::DevToolsHttpHandlerDelegate {
 public:
  explicit XWalkDevToolsDelegate(XWalkBrowserContext* browser_context);
  ~XWalkDevToolsDelegate() override;

  // devtools_http_handler::DevToolsHttpHandlerDelegate implementation.
  std::string GetDiscoveryPageHTML() override;
  std::string GetFrontendResource(const std::string& path) override;
  std::string GetPageThumbnailData(const GURL& url) override;
  content::DevToolsExternalAgentProxyDelegate*
      HandleWebSocketConnection(const std::string& path) override;

 private:
  XWalkBrowserContext* browser_context_;

  DISALLOW_COPY_AND_ASSIGN(XWalkDevToolsDelegate);
};

XWalkDevToolsDelegate::XWalkDevToolsDelegate(
    XWalkBrowserContext* browser_context)
    : browser_context_(browser_context) {
  devtools_discovery::DevToolsDiscoveryManager::GetInstance()->
      SetCreateCallback(base::Bind(&CreateNewShellTarget,
                                   base::Unretained(browser_context_)));
}

XWalkDevToolsDelegate::~XWalkDevToolsDelegate() {
  devtools_discovery::DevToolsDiscoveryManager::GetInstance()->
      SetCreateCallback(
          devtools_discovery::DevToolsDiscoveryManager::CreateCallback());
}

std::string XWalkDevToolsDelegate::GetDiscoveryPageHTML() {
  return ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_DEVTOOLS_FRONTEND_PAGE_HTML).as_string();
}

std::string XWalkDevToolsDelegate::GetFrontendResource(
    const std::string& path) {
  return content::DevToolsFrontendHost::GetFrontendResource(path).as_string();
}

std::string XWalkDevToolsDelegate::GetPageThumbnailData(const GURL& url) {
  return std::string();
}

content::DevToolsExternalAgentProxyDelegate*
XWalkDevToolsDelegate::HandleWebSocketConnection(const std::string& path) {
  return nullptr;
}

}  // namespace

DevToolsHttpHandler* XWalkDevToolsManagerDelegate::CreateHttpHandler(
    XWalkBrowserContext* browser_context) {
  std::string frontend_url;
  return new DevToolsHttpHandler(
      CreateSocketFactory(GetInspectorPort()),
      frontend_url,
      new XWalkDevToolsDelegate(browser_context),
      base::FilePath(),
      base::FilePath(),
      std::string(),
      xwalk::GetUserAgent());
}

XWalkDevToolsManagerDelegate::~XWalkDevToolsManagerDelegate() {
}

base::DictionaryValue* XWalkDevToolsManagerDelegate::HandleCommand(
    content::DevToolsAgentHost* agent_host,
    base::DictionaryValue* command) {
  return nullptr;
}

}  // namespace xwalk
