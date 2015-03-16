// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/devtools/remote_debugging_server.h"

#include "content/public/browser/devtools_http_handler.h"
#include "net/base/net_errors.h"
#include "net/socket/tcp_server_socket.h"
#include "xwalk/runtime/browser/devtools/xwalk_devtools_delegate.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"

namespace xwalk {

class TCPServerSocketFactory
    : public content::DevToolsHttpHandler::ServerSocketFactory {
 public:
  TCPServerSocketFactory(
    const std::string& address, int port, int backlog)
    : address_(address)
    , backlog_(backlog)
    , port_(port) {
  }

 private:
  // content::DevToolsHttpHandler::ServerSocketFactory.
  scoped_ptr<net::ServerSocket> CreateForHttpServer() override {
    scoped_ptr<net::ServerSocket> socket(
        new net::TCPServerSocket(NULL, net::NetLog::Source()));
    if (socket->ListenWithAddressAndPort(address_, port_, backlog_) != net::OK)
      return scoped_ptr<net::ServerSocket>();
    return socket;
  }
  std::string address_;
  int backlog_;
  int port_;
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
  devtools_http_handler_.reset(content::DevToolsHttpHandler::Start(
      factory.Pass(),
      frontend_url,
      new XWalkDevToolsHttpHandlerDelegate(),
      output_dir));
  port_ = port;
}

RemoteDebuggingServer::~RemoteDebuggingServer() {
}

}  // namespace xwalk
