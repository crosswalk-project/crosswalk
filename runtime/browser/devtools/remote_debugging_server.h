// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_DEVTOOLS_REMOTE_DEBUGGING_SERVER_H_
#define XWALK_RUNTIME_BROWSER_DEVTOOLS_REMOTE_DEBUGGING_SERVER_H_

#include <memory>
#include <string>

#include "base/macros.h"

namespace devtools_http_handler {
class DevToolsHttpHandler;
}

namespace xwalk {

class XWalkBrowserContext;

class RemoteDebuggingServer {
 public:
  RemoteDebuggingServer(XWalkBrowserContext* browser_context,
                        const std::string& ip,
                        int port,
                        const std::string& frontend_url);

  virtual ~RemoteDebuggingServer();
  int port() { return port_; }

 private:
  std::unique_ptr<devtools_http_handler::DevToolsHttpHandler> devtools_http_handler_;
  int port_;
  DISALLOW_COPY_AND_ASSIGN(RemoteDebuggingServer);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_DEVTOOLS_REMOTE_DEBUGGING_SERVER_H_
