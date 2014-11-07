// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_DEV_TOOLS_SERVER_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_DEV_TOOLS_SERVER_H_

#include <jni.h>
#include <string>
#include "base/basictypes.h"
#include "net/socket/unix_domain_server_socket_posix.h"

namespace content {
class DevToolsHttpHandler;
}

namespace xwalk {

// This class controls Developer Tools remote debugging server.
class XWalkDevToolsServer {
 public:
  explicit XWalkDevToolsServer(const std::string& socket_name);
  ~XWalkDevToolsServer();

  // Opens linux abstract socket to be ready for remote debugging.
  void Start(bool allow_debug_permission, bool allow_socket_access);

  // Closes debugging socket, stops debugging.
  void Stop();

  bool IsStarted() const;

  void AllowConnectionFromUid(uid_t uid);

 private:
  bool CanUserConnectToDevTools(
    const net::UnixDomainServerSocket::Credentials& credentials);

  std::string socket_name_;
  content::DevToolsHttpHandler* protocol_handler_;
  bool allow_debug_permission_;
  bool allow_socket_access_;

  DISALLOW_COPY_AND_ASSIGN(XWalkDevToolsServer);
};

bool RegisterXWalkDevToolsServer(JNIEnv* env);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_DEV_TOOLS_SERVER_H_
