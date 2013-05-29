// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RUNTIME_BROWSER_DEVTOOLS_REMOTE_DEBUGGING_SERVER_H_
#define CAMEO_SRC_RUNTIME_BROWSER_DEVTOOLS_REMOTE_DEBUGGING_SERVER_H_

#include <string>

#include "base/basictypes.h"

namespace content {
class DevToolsHttpHandler;
}

namespace cameo {

class RuntimeContext;

class RemoteDebuggingServer {
 public:
  RemoteDebuggingServer(RuntimeContext* runtime_context,
                        const std::string& ip,
                        int port,
                        const std::string& frontend_url);

  virtual ~RemoteDebuggingServer();

  content::DevToolsHttpHandler* devtools_http_handler() const {
    return devtools_http_handler_;
  }

 private:
  content::DevToolsHttpHandler* devtools_http_handler_;
  DISALLOW_COPY_AND_ASSIGN(RemoteDebuggingServer);
};

}  // namespace cameo

#endif  // CAMEO_SRC_RUNTIME_BROWSER_DEVTOOLS_REMOTE_DEBUGGING_SERVER_H_
