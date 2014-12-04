// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/devtools/remote_debugging_server.h"

#include "content/public/browser/devtools_http_handler.h"
#include "net/socket/tcp_server_socket.h"
#include "xwalk/runtime/browser/devtools/xwalk_devtools_delegate.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"

namespace xwalk {

class TCPServerSocketFactory
    : public content::DevToolsHttpHandler::ServerSocketFactory {
 public:
  TCPServerSocketFactory(const std::string& address, int port, int backlog)
      : content::DevToolsHttpHandler::ServerSocketFactory(
            address, port, backlog) {}

 private:
  // content::DevToolsHttpHandler::ServerSocketFactory.
  scoped_ptr<net::ServerSocket> Create() const override {
    return scoped_ptr<net::ServerSocket>(
        new net::TCPServerSocket(NULL, net::NetLog::Source()));
  }

  DISALLOW_COPY_AND_ASSIGN(TCPServerSocketFactory);
};

RemoteDebuggingServer::RemoteDebuggingServer(
    XWalkBrowserContext* browser_context,
    const std::string& ip,
    int port,
    const std::string& frontend_url) {
  base::FilePath output_dir;
  scoped_ptr<content::DevToolsHttpHandler::ServerSocketFactory> factory(
      new TCPServerSocketFactory(ip, port, 1));
  devtools_http_handler_ = content::DevToolsHttpHandler::Start(
      factory.Pass(),
      frontend_url,
      new XWalkDevToolsHttpHandlerDelegate(),
      output_dir);
}

RemoteDebuggingServer::~RemoteDebuggingServer() {
  devtools_http_handler_->Stop();
}

}  // namespace xwalk
