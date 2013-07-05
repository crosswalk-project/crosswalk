// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/runtime/browser/devtools/remote_debugging_server.h"

#include "cameo/runtime/browser/devtools/xwalk_devtools_delegate.h"
#include "cameo/runtime/browser/runtime_context.h"
#include "content/public/browser/devtools_http_handler.h"
#include "net/socket/tcp_listen_socket.h"

namespace cameo {

RemoteDebuggingServer::RemoteDebuggingServer(
    RuntimeContext* runtime_context,
    const std::string& ip,
    int port,
    const std::string& frontend_url) {
  devtools_http_handler_ = content::DevToolsHttpHandler::Start(
      new net::TCPListenSocketFactory(ip, port),
      frontend_url,
      new XWalkDevToolsDelegate(runtime_context));
}

RemoteDebuggingServer::~RemoteDebuggingServer() {
  devtools_http_handler_->Stop();
}

}  // namespace cameo
