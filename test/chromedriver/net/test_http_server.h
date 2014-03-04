// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_NET_TEST_HTTP_SERVER_H_
#define CHROME_TEST_CHROMEDRIVER_NET_TEST_HTTP_SERVER_H_

#include <set>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "net/server/http_server.h"
#include "url/gurl.h"

namespace base {
class WaitableEvent;
}

// HTTP server for web socket testing purposes that runs on its own thread.
// All public methods are thread safe and may be called on any thread, unless
// noted otherwise.
class TestHttpServer : public net::HttpServer::Delegate {
 public:
  enum WebSocketRequestAction {
    kAccept,
    kNotFound,
    kClose,
  };

  enum WebSocketMessageAction {
    kEchoMessage,
    kCloseOnMessage
  };

  // Creates an http server. By default it accepts WebSockets and echoes
  // WebSocket messages back.
  TestHttpServer();
  virtual ~TestHttpServer();

  // Starts the server. Returns whether it was started successfully.
  bool Start();

  // Stops the server. May be called multiple times.
  void Stop();

  // Waits until all open connections are closed. Returns true if all
  // connections are closed, or false if a timeout is exceeded.
  bool WaitForConnectionsToClose();

  // Sets the action to perform when receiving a WebSocket connect request.
  void SetRequestAction(WebSocketRequestAction action);

  // Sets the action to perform when receiving a WebSocket message.
  void SetMessageAction(WebSocketMessageAction action);

  // Returns the web socket URL that points to the server.
  GURL web_socket_url() const;

  // Overridden from net::HttpServer::Delegate:
  virtual void OnHttpRequest(int connection_id,
                             const net::HttpServerRequestInfo& info) OVERRIDE {}
  virtual void OnWebSocketRequest(
      int connection_id,
      const net::HttpServerRequestInfo& info) OVERRIDE;
  virtual void OnWebSocketMessage(int connection_id,
                                  const std::string& data) OVERRIDE;
  virtual void OnClose(int connection_id) OVERRIDE;

 private:
  void StartOnServerThread(bool* success, base::WaitableEvent* event);
  void StopOnServerThread(base::WaitableEvent* event);

  base::Thread thread_;

  // Access only on the server thread.
  scoped_refptr<net::HttpServer> server_;

  // Access only on the server thread.
  std::set<int> connections_;

  base::WaitableEvent all_closed_event_;

  // Protects |web_socket_url_|.
  mutable base::Lock url_lock_;
  GURL web_socket_url_;

  // Protects the action flags.
  base::Lock action_lock_;
  WebSocketRequestAction request_action_;
  WebSocketMessageAction message_action_;

  DISALLOW_COPY_AND_ASSIGN(TestHttpServer);
};

#endif  // CHROME_TEST_CHROMEDRIVER_NET_TEST_HTTP_SERVER_H_
