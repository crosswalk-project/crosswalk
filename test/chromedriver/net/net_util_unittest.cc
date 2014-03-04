// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/location.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/stringprintf.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "chrome/test/chromedriver/net/net_util.h"
#include "chrome/test/chromedriver/net/url_request_context_getter.h"
#include "net/base/ip_endpoint.h"
#include "net/base/net_errors.h"
#include "net/server/http_server.h"
#include "net/server/http_server_request_info.h"
#include "net/socket/tcp_listen_socket.h"
#include "net/url_request/url_request_context_getter.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class FetchUrlTest : public testing::Test,
                     public net::HttpServer::Delegate {
 public:
  FetchUrlTest()
      : io_thread_("io"),
        response_(kSendHello) {
    base::Thread::Options options(base::MessageLoop::TYPE_IO, 0);
    CHECK(io_thread_.StartWithOptions(options));
    context_getter_ = new URLRequestContextGetter(
        io_thread_.message_loop_proxy());
    base::WaitableEvent event(false, false);
    io_thread_.message_loop_proxy()->PostTask(
        FROM_HERE,
        base::Bind(&FetchUrlTest::InitOnIO,
                   base::Unretained(this), &event));
    event.Wait();
  }

  virtual ~FetchUrlTest() {
    base::WaitableEvent event(false, false);
    io_thread_.message_loop_proxy()->PostTask(
        FROM_HERE,
        base::Bind(&FetchUrlTest::DestroyServerOnIO,
                   base::Unretained(this), &event));
    event.Wait();
  }

  void InitOnIO(base::WaitableEvent* event) {
    net::TCPListenSocketFactory factory("127.0.0.1", 0);
    server_ = new net::HttpServer(factory, this);
    net::IPEndPoint address;
    CHECK_EQ(net::OK, server_->GetLocalAddress(&address));
    server_url_ = base::StringPrintf("http://127.0.0.1:%d", address.port());
    event->Signal();
  }

  void DestroyServerOnIO(base::WaitableEvent* event) {
    server_ = NULL;
    event->Signal();
  }

  // Overridden from net::HttpServer::Delegate:
  virtual void OnHttpRequest(int connection_id,
                             const net::HttpServerRequestInfo& info) OVERRIDE {
    switch (response_) {
      case kSendHello:
        server_->Send200(connection_id, "hello", "text/plain");
        break;
      case kSend404:
        server_->Send404(connection_id);
        break;
      case kClose:
        // net::HttpServer doesn't allow us to close connection during callback.
        base::MessageLoop::current()->PostTask(
            FROM_HERE,
            base::Bind(&net::HttpServer::Close, server_, connection_id));
        break;
      default:
        break;
    }
  }

  virtual void OnWebSocketRequest(
      int connection_id,
      const net::HttpServerRequestInfo& info) OVERRIDE {}
  virtual void OnWebSocketMessage(int connection_id,
                                  const std::string& data) OVERRIDE {}
  virtual void OnClose(int connection_id) OVERRIDE {}

 protected:
  enum ServerResponse {
    kSendHello = 0,
    kSend404,
    kClose,
  };

  base::Thread io_thread_;
  ServerResponse response_;
  scoped_refptr<net::HttpServer> server_;
  scoped_refptr<URLRequestContextGetter> context_getter_;
  std::string server_url_;
};

}  // namespace

TEST_F(FetchUrlTest, Http200) {
  std::string response("stuff");
  ASSERT_TRUE(FetchUrl(server_url_, context_getter_.get(), &response));
  ASSERT_STREQ("hello", response.c_str());
}

TEST_F(FetchUrlTest, HttpNon200) {
  response_ = kSend404;
  std::string response("stuff");
  ASSERT_FALSE(FetchUrl(server_url_, context_getter_.get(), &response));
  ASSERT_STREQ("stuff", response.c_str());
}

TEST_F(FetchUrlTest, ConnectionClose) {
  response_ = kClose;
  std::string response("stuff");
  ASSERT_FALSE(FetchUrl(server_url_, context_getter_.get(), &response));
  ASSERT_STREQ("stuff", response.c_str());
}

TEST_F(FetchUrlTest, NoServer) {
  std::string response("stuff");
  ASSERT_FALSE(
      FetchUrl("http://localhost:33333", context_getter_.get(), &response));
  ASSERT_STREQ("stuff", response.c_str());
}
