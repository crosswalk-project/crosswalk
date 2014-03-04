// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/location.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/run_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread.h"
#include "base/time/time.h"
#include "chrome/test/chromedriver/net/test_http_server.h"
#include "chrome/test/chromedriver/net/websocket.h"
#include "net/url_request/url_request_test_util.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace {

void OnConnectFinished(base::RunLoop* run_loop, int* save_error, int error) {
  *save_error = error;
  run_loop->Quit();
}

void RunPending(base::MessageLoop* loop) {
  base::RunLoop run_loop;
  loop->PostTask(FROM_HERE, run_loop.QuitClosure());
  run_loop.Run();
}

class Listener : public WebSocketListener {
 public:
  explicit Listener(const std::vector<std::string>& messages)
      : messages_(messages) {}

  virtual ~Listener() {
    EXPECT_TRUE(messages_.empty());
  }

  virtual void OnMessageReceived(const std::string& message) OVERRIDE {
    ASSERT_TRUE(messages_.size());
    EXPECT_EQ(messages_[0], message);
    messages_.erase(messages_.begin());
    if (messages_.empty())
      base::MessageLoop::current()->Quit();
  }

  virtual void OnClose() OVERRIDE {
    EXPECT_TRUE(false);
  }

 private:
  std::vector<std::string> messages_;
};

class CloseListener : public WebSocketListener {
 public:
  explicit CloseListener(base::RunLoop* run_loop)
      : run_loop_(run_loop) {}

  virtual ~CloseListener() {
    EXPECT_FALSE(run_loop_);
  }

  virtual void OnMessageReceived(const std::string& message) OVERRIDE {}

  virtual void OnClose() OVERRIDE {
    EXPECT_TRUE(run_loop_);
    if (run_loop_)
      run_loop_->Quit();
    run_loop_ = NULL;
  }

 private:
  base::RunLoop* run_loop_;
};

class WebSocketTest : public testing::Test {
 public:
  WebSocketTest() {}
  virtual ~WebSocketTest() {}

  virtual void SetUp() OVERRIDE {
    ASSERT_TRUE(server_.Start());
  }

  virtual void TearDown() OVERRIDE {
    server_.Stop();
  }

 protected:
  scoped_ptr<WebSocket> CreateWebSocket(const GURL& url,
                                        WebSocketListener* listener) {
    int error;
    scoped_ptr<WebSocket> sock(new WebSocket(url, listener));
    base::RunLoop run_loop;
    sock->Connect(base::Bind(&OnConnectFinished, &run_loop, &error));
    loop_.PostDelayedTask(
        FROM_HERE, run_loop.QuitClosure(),
        base::TimeDelta::FromSeconds(10));
    run_loop.Run();
    if (error == net::OK)
      return sock.Pass();
    return scoped_ptr<WebSocket>();
  }

  scoped_ptr<WebSocket> CreateConnectedWebSocket(WebSocketListener* listener) {
    return CreateWebSocket(server_.web_socket_url(), listener);
  }

  void SendReceive(const std::vector<std::string>& messages) {
    Listener listener(messages);
    scoped_ptr<WebSocket> sock(CreateConnectedWebSocket(&listener));
    ASSERT_TRUE(sock);
    for (size_t i = 0; i < messages.size(); ++i) {
      ASSERT_TRUE(sock->Send(messages[i]));
    }
    base::RunLoop run_loop;
    loop_.PostDelayedTask(
        FROM_HERE, run_loop.QuitClosure(),
        base::TimeDelta::FromSeconds(10));
    run_loop.Run();
  }

  base::MessageLoopForIO loop_;
  TestHttpServer server_;
};

}  // namespace

TEST_F(WebSocketTest, CreateDestroy) {
  CloseListener listener(NULL);
  WebSocket sock(GURL("ws://127.0.0.1:2222"), &listener);
}

TEST_F(WebSocketTest, Connect) {
  CloseListener listener(NULL);
  ASSERT_TRUE(CreateWebSocket(server_.web_socket_url(), &listener));
  RunPending(&loop_);
  ASSERT_TRUE(server_.WaitForConnectionsToClose());
}

TEST_F(WebSocketTest, ConnectNoServer) {
  CloseListener listener(NULL);
  ASSERT_FALSE(CreateWebSocket(GURL("ws://127.0.0.1:33333"), NULL));
}

TEST_F(WebSocketTest, Connect404) {
  server_.SetRequestAction(TestHttpServer::kNotFound);
  CloseListener listener(NULL);
  ASSERT_FALSE(CreateWebSocket(server_.web_socket_url(), NULL));
  RunPending(&loop_);
  ASSERT_TRUE(server_.WaitForConnectionsToClose());
}

TEST_F(WebSocketTest, ConnectServerClosesConn) {
  server_.SetRequestAction(TestHttpServer::kClose);
  CloseListener listener(NULL);
  ASSERT_FALSE(CreateWebSocket(server_.web_socket_url(), &listener));
}

TEST_F(WebSocketTest, CloseOnReceive) {
  server_.SetMessageAction(TestHttpServer::kCloseOnMessage);
  base::RunLoop run_loop;
  CloseListener listener(&run_loop);
  scoped_ptr<WebSocket> sock(CreateConnectedWebSocket(&listener));
  ASSERT_TRUE(sock);
  ASSERT_TRUE(sock->Send("hi"));
  loop_.PostDelayedTask(
      FROM_HERE, run_loop.QuitClosure(),
      base::TimeDelta::FromSeconds(10));
  run_loop.Run();
}

TEST_F(WebSocketTest, CloseOnSend) {
  base::RunLoop run_loop;
  CloseListener listener(&run_loop);
  scoped_ptr<WebSocket> sock(CreateConnectedWebSocket(&listener));
  ASSERT_TRUE(sock);
  server_.Stop();

  sock->Send("hi");
  loop_.PostDelayedTask(
      FROM_HERE, run_loop.QuitClosure(),
      base::TimeDelta::FromSeconds(10));
  run_loop.Run();
  ASSERT_FALSE(sock->Send("hi"));
}

TEST_F(WebSocketTest, SendReceive) {
  std::vector<std::string> messages;
  messages.push_back("hello");
  SendReceive(messages);
}

TEST_F(WebSocketTest, SendReceiveLarge) {
  std::vector<std::string> messages;
  messages.push_back(std::string(10 << 20, 'a'));
  SendReceive(messages);
}

TEST_F(WebSocketTest, SendReceiveMultiple) {
  std::vector<std::string> messages;
  messages.push_back("1");
  messages.push_back("2");
  messages.push_back("3");
  SendReceive(messages);
}
