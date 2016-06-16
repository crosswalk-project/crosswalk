// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_RAW_SOCKET_UDP_SOCKET_OBJECT_H_
#define XWALK_SYSAPPS_RAW_SOCKET_UDP_SOCKET_OBJECT_H_

#include <string>

#include "net/base/address_list.h"
#include "net/base/io_buffer.h"
#include "net/dns/single_request_host_resolver.h"
#include "net/udp/udp_socket.h"
#include "xwalk/sysapps/raw_socket/raw_socket_object.h"

namespace xwalk {
namespace sysapps {

class UDPSocketObject : public RawSocketObject {
 public:
  UDPSocketObject();
  ~UDPSocketObject() override;

 private:
  void DoRead();

  // JavaScript function handlers.
  void OnInit(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnClose(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnSuspend(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnResume(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnJoinMulticast(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnLeaveMulticast(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnSendString(std::unique_ptr<XWalkExtensionFunctionInfo> info);

  // net::UDPSocket callbacks.
  void OnRead(int status);
  void OnWrite(int status);

  // net::SingleRequestHostResolver callbacks.
  void OnConnectionOpen(int status);
  void OnSend(int status);

  bool has_write_pending_;
  bool is_suspended_;
  bool is_reading_;

  scoped_refptr<net::IOBuffer> read_buffer_;
  scoped_refptr<net::IOBuffer> write_buffer_;
  std::unique_ptr<net::UDPSocket> socket_;

  size_t write_buffer_size_;

  std::unique_ptr<net::HostResolver> resolver_;
  std::unique_ptr<net::SingleRequestHostResolver> single_resolver_;
  net::AddressList addresses_;
  net::IPEndPoint from_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_RAW_SOCKET_UDP_SOCKET_OBJECT_H_
