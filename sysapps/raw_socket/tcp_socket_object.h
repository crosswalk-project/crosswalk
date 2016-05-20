// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_RAW_SOCKET_TCP_SOCKET_OBJECT_H_
#define XWALK_SYSAPPS_RAW_SOCKET_TCP_SOCKET_OBJECT_H_

#include <string>
#include "net/dns/single_request_host_resolver.h"
#include "net/base/io_buffer.h"
#include "net/socket/tcp_client_socket.h"
#include "xwalk/sysapps/raw_socket/raw_socket_object.h"

namespace xwalk {
namespace sysapps {

class TCPSocketObject : public RawSocketObject {
 public:
  TCPSocketObject();
  explicit TCPSocketObject(std::unique_ptr<net::StreamSocket> socket);
  ~TCPSocketObject() override;

 private:
  void RegisterHandlers();
  void DoRead();

  // JavaScript function handlers.
  void OnInit(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnClose(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnHalfClose(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnSuspend(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnResume(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnSendString(std::unique_ptr<XWalkExtensionFunctionInfo> info);

  // net::TCPClientSocket callbacks.
  void OnConnect(int status);
  void OnRead(int status);
  void OnWrite(int status);

  // net::SingleRequestHostResolver callbacks.
  void OnResolved(int status);

  bool has_write_pending_;
  bool is_suspended_;
  bool is_half_closed_;

  scoped_refptr<net::IOBuffer> read_buffer_;
  scoped_refptr<net::IOBuffer> write_buffer_;
  std::unique_ptr<net::StreamSocket> socket_;

  std::unique_ptr<net::HostResolver> resolver_;
  std::unique_ptr<net::SingleRequestHostResolver> single_resolver_;
  net::AddressList addresses_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_RAW_SOCKET_TCP_SOCKET_OBJECT_H_
