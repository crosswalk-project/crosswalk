// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/net/test_http_server.h"

#include "base/bind.h"
#include "base/location.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "net/base/ip_endpoint.h"
#include "net/base/net_errors.h"
#include "net/server/http_server_request_info.h"
#include "net/socket/tcp_listen_socket.h"
#include "testing/gtest/include/gtest/gtest.h"

TestHttpServer::TestHttpServer()
    : thread_("ServerThread"),
      all_closed_event_(false, true),
      request_action_(kAccept),
      message_action_(kEchoMessage) {
}

TestHttpServer::~TestHttpServer() {
}

bool TestHttpServer::Start() {
  base::Thread::Options options(base::MessageLoop::TYPE_IO, 0);
  bool thread_started = thread_.StartWithOptions(options);
  EXPECT_TRUE(thread_started);
  if (!thread_started)
    return false;
  bool success;
  base::WaitableEvent event(false, false);
  thread_.message_loop_proxy()->PostTask(
      FROM_HERE,
      base::Bind(&TestHttpServer::StartOnServerThread,
                 base::Unretained(this), &success, &event));
  event.Wait();
  return success;
}

void TestHttpServer::Stop() {
  if (!thread_.IsRunning())
    return;
  base::WaitableEvent event(false, false);
  thread_.message_loop_proxy()->PostTask(
      FROM_HERE,
      base::Bind(&TestHttpServer::StopOnServerThread,
                 base::Unretained(this), &event));
  event.Wait();
  thread_.Stop();
}

bool TestHttpServer::WaitForConnectionsToClose() {
  return all_closed_event_.TimedWait(base::TimeDelta::FromSeconds(10));
}

void TestHttpServer::SetRequestAction(WebSocketRequestAction action) {
  base::AutoLock lock(action_lock_);
  request_action_ = action;
}

void TestHttpServer::SetMessageAction(WebSocketMessageAction action) {
  base::AutoLock lock(action_lock_);
  message_action_ = action;
}

GURL TestHttpServer::web_socket_url() const {
  base::AutoLock lock(url_lock_);
  return web_socket_url_;
}

void TestHttpServer::OnWebSocketRequest(
    int connection_id,
    const net::HttpServerRequestInfo& info) {
  WebSocketRequestAction action;
  {
    base::AutoLock lock(action_lock_);
    action = request_action_;
  }
  connections_.insert(connection_id);
  all_closed_event_.Reset();

  switch (action) {
    case kAccept:
      server_->AcceptWebSocket(connection_id, info);
      break;
    case kNotFound:
      server_->Send404(connection_id);
      break;
    case kClose:
      // net::HttpServer doesn't allow us to close connection during callback.
      base::MessageLoop::current()->PostTask(
          FROM_HERE,
          base::Bind(&net::HttpServer::Close, server_, connection_id));
      break;
  }
}

void TestHttpServer::OnWebSocketMessage(int connection_id,
                                        const std::string& data) {
  WebSocketMessageAction action;
  {
    base::AutoLock lock(action_lock_);
    action = message_action_;
  }
  switch (action) {
    case kEchoMessage:
      server_->SendOverWebSocket(connection_id, data);
      break;
    case kCloseOnMessage:
      // net::HttpServer doesn't allow us to close connection during callback.
      base::MessageLoop::current()->PostTask(
          FROM_HERE,
          base::Bind(&net::HttpServer::Close, server_, connection_id));
      break;
  }
}

void TestHttpServer::OnClose(int connection_id) {
  connections_.erase(connection_id);
  if (connections_.empty())
    all_closed_event_.Signal();
}

void TestHttpServer::StartOnServerThread(bool* success,
                                         base::WaitableEvent* event) {
  net::TCPListenSocketFactory factory("127.0.0.1", 0);
  server_ = new net::HttpServer(factory, this);

  net::IPEndPoint address;
  int error = server_->GetLocalAddress(&address);
  EXPECT_EQ(net::OK, error);
  if (error == net::OK) {
    base::AutoLock lock(url_lock_);
    web_socket_url_ = GURL(base::StringPrintf("ws://127.0.0.1:%d",
                                              address.port()));
  } else {
    server_ = NULL;
  }
  *success = server_.get();
  event->Signal();
}

void TestHttpServer::StopOnServerThread(base::WaitableEvent* event) {
  if (server_.get())
    server_ = NULL;
  event->Signal();
}
