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
  virtual ~TCPServerSocketObject();

 private:
  void DoAccept();

  // EventTarget implementation.
  virtual void StartEvent(const std::string& type) OVERRIDE;
  virtual void StopEvent(const std::string& type) OVERRIDE;

  // JavaScript function handlers.
  void OnInit(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnClose(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnSuspend(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnResume(scoped_ptr<XWalkExtensionFunctionInfo> info);

  // net::TCPServerSocket callbacks.
  void OnAccept(int status);

  bool is_suspended_;
  bool is_accepting_;

  scoped_ptr<net::TCPServerSocket> socket_;
  scoped_ptr<net::StreamSocket> accepted_socket_;

  RawSocketInstance* instance_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_RAW_SOCKET_TCP_SERVER_SOCKET_OBJECT_H_
