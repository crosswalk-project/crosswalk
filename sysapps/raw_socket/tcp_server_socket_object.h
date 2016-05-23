// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_RAW_SOCKET_TCP_SERVER_SOCKET_OBJECT_H_
#define XWALK_SYSAPPS_RAW_SOCKET_TCP_SERVER_SOCKET_OBJECT_H_

#include <string>
#include "net/socket/tcp_server_socket.h"
#include "xwalk/sysapps/common/event_target.h"
#include "xwalk/sysapps/raw_socket/raw_socket_extension.h"
#include "xwalk/sysapps/raw_socket/raw_socket_object.h"

namespace xwalk {
namespace sysapps {

class BindingObjectStore;

class TCPServerSocketObject : public RawSocketObject {
 public:
  explicit TCPServerSocketObject(RawSocketInstance* instance);
  ~TCPServerSocketObject() override;

 private:
  void DoAccept();

  // EventTarget implementation.
  void StartEvent(const std::string& type) override;
  void StopEvent(const std::string& type) override;

  // JavaScript function handlers.
  void OnInit(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnClose(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnSuspend(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnResume(std::unique_ptr<XWalkExtensionFunctionInfo> info);

  // net::TCPServerSocket callbacks.
  void OnAccept(int status);

  bool is_suspended_;
  bool is_accepting_;

  std::unique_ptr<net::TCPServerSocket> socket_;
  std::unique_ptr<net::StreamSocket> accepted_socket_;

  RawSocketInstance* instance_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_RAW_SOCKET_TCP_SERVER_SOCKET_OBJECT_H_
