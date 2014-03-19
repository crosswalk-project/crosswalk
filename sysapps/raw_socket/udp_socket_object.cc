// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/raw_socket/udp_socket_object.h"

#include <string.h>

#include "base/logging.h"
#include "net/base/net_errors.h"
#include "xwalk/sysapps/raw_socket/udp_socket.h"

using namespace xwalk::jsapi::udp_socket; // NOLINT
using namespace xwalk::jsapi::raw_socket; // NOLINT

namespace {

const unsigned kBufferSize = 4096;

}  // namespace

namespace xwalk {
namespace sysapps {

UDPSocketObject::UDPSocketObject()
    : has_write_pending_(false),
      is_suspended_(false),
      is_reading_(false),
      resolver_(net::HostResolver::CreateDefaultResolver(NULL)),
      read_buffer_(new net::IOBuffer(kBufferSize)),
      write_buffer_(new net::IOBuffer(kBufferSize)),
      single_resolver_(new net::SingleRequestHostResolver(resolver_.get())) {
  handler_.Register("init",
      base::Bind(&UDPSocketObject::OnInit, base::Unretained(this)));
  handler_.Register("_close",
      base::Bind(&UDPSocketObject::OnClose, base::Unretained(this)));
  handler_.Register("suspend",
      base::Bind(&UDPSocketObject::OnSuspend, base::Unretained(this)));
  handler_.Register("resume",
      base::Bind(&UDPSocketObject::OnResume, base::Unretained(this)));
  handler_.Register("joinMulticast",
      base::Bind(&UDPSocketObject::OnJoinMulticast, base::Unretained(this)));
  handler_.Register("leaveMulticast",
      base::Bind(&UDPSocketObject::OnLeaveMulticast, base::Unretained(this)));
  handler_.Register("_sendString",
      base::Bind(&UDPSocketObject::OnSendString, base::Unretained(this)));
}

UDPSocketObject::~UDPSocketObject() {}

void UDPSocketObject::DoRead() {
  if (!socket_->is_connected())
    return;

  is_reading_ = true;

  int ret = socket_->RecvFrom(read_buffer_,
                              kBufferSize,
                              &from_,
                              base::Bind(&UDPSocketObject::OnRead,
                                         base::Unretained(this)));

  if (ret > 0)
    OnRead(ret);
}

void UDPSocketObject::OnInit(scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<Init::Params> params(Init::Params::Create(*info->arguments()));
  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    setReadyState(READY_STATE_CLOSED);
    DispatchEvent("error");
    return;
  }

  socket_.reset(new net::UDPSocket(net::DatagramSocket::DEFAULT_BIND,
                                   net::RandIntCallback(),
                                   NULL,
                                   net::NetLog::Source()));

  if (!params->options) {
    OnConnectionOpen(net::OK);
    return;
  }

  if (!params->options->local_address.empty()) {
    net::IPAddressNumber ip_number;
    if (!net::ParseIPLiteralToNumber(params->options->local_address,
                                     &ip_number)) {
      LOG(WARNING) << "Invalid IP address " << params->options->local_address;
      setReadyState(READY_STATE_CLOSED);
      DispatchEvent("error");
      return;
    }

    if (params->options->address_reuse)
      socket_->AllowAddressReuse();

    net::IPEndPoint end_point(ip_number, params->options->local_port);
    if (socket_->Bind(end_point) != net::OK) {
      LOG(WARNING) << "Can't bind to " << end_point.ToString();
      setReadyState(READY_STATE_CLOSED);
      DispatchEvent("error");
      return;
    }

    DoRead();
    OnConnectionOpen(net::OK);
    return;
  }

  if (params->options->remote_address.empty() ||
      !params->options->remote_port) {
    OnConnectionOpen(net::OK);
    return;
  }

  net::HostResolver::RequestInfo request_info(net::HostPortPair(
      params->options->remote_address, params->options->remote_port));

  int ret = single_resolver_->Resolve(
      request_info,
      net::DEFAULT_PRIORITY,
      &addresses_,
      base::Bind(&UDPSocketObject::OnConnectionOpen,
                 base::Unretained(this)),
      net::BoundNetLog());

  if (ret != net::ERR_IO_PENDING)
    OnConnectionOpen(ret);
}

void UDPSocketObject::OnClose(scoped_ptr<XWalkExtensionFunctionInfo> info) {
  socket_.reset();
}

void UDPSocketObject::OnSuspend(scoped_ptr<XWalkExtensionFunctionInfo> info) {
  is_suspended_ = true;
}

void UDPSocketObject::OnResume(scoped_ptr<XWalkExtensionFunctionInfo> info) {
  is_suspended_ = false;
}

void UDPSocketObject::OnJoinMulticast(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
}

void UDPSocketObject::OnLeaveMulticast(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
}

void UDPSocketObject::OnSendString(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  if (!socket_ || has_write_pending_)
    return;

  scoped_ptr<SendDOMString::Params>
      params(SendDOMString::Params::Create(*info->arguments()));
  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }

  if (params->data.size() > kBufferSize) {
    LOG(WARNING) << "Write data bigger than the write buffer.";
    return;
  }

  write_buffer_size_ = params->data.size();
  memcpy(write_buffer_->data(), params->data.data(), write_buffer_size_);

  if (!params->remote_address || !*params->remote_port) {
    OnSend(net::OK);
    return;
  }

  net::HostResolver::RequestInfo request_info(net::HostPortPair(
      *params->remote_address, *params->remote_port));

  int ret = single_resolver_->Resolve(
      request_info,
      net::DEFAULT_PRIORITY,
      &addresses_,
      base::Bind(&UDPSocketObject::OnSend,
                 base::Unretained(this)),
      net::BoundNetLog());

  if (ret != net::ERR_IO_PENDING)
    OnSend(ret);
  else
    has_write_pending_ = true;
}

void UDPSocketObject::OnRead(int status) {
  // No data means the other side has
  // disconnected the socket.
  if (status == 0) {
    setReadyState(READY_STATE_CLOSED);
    DispatchEvent("close");
    return;
  }

  scoped_ptr<base::Value> data(base::BinaryValue::CreateWithCopiedBuffer(
      static_cast<char*>(read_buffer_->data()), status));

  UDPMessageEvent event;
  event.data = std::string(read_buffer_->data(), status);
  event.remote_port = from_.port();
  event.remote_address = from_.ToStringWithoutPort();

  scoped_ptr<base::ListValue> eventData(new base::ListValue);
  eventData->Append(event.ToValue().release());

  if (!is_suspended_)
    DispatchEvent("message", eventData.Pass());

  DoRead();
}

void UDPSocketObject::OnWrite(int status) {
  has_write_pending_ = false;
  DispatchEvent("drain");
}

void UDPSocketObject::OnConnectionOpen(int status) {
  if (status != net::OK) {
    setReadyState(READY_STATE_CLOSED);
    DispatchEvent("error");
    return;
  }

  setReadyState(READY_STATE_OPEN);
  DispatchEvent("open");
}

void UDPSocketObject::OnSend(int status) {
  if (status != net::OK || addresses_.empty()) {
    setReadyState(READY_STATE_CLOSED);
    DispatchEvent("error");
    return;
  }

  if (!socket_->is_connected()) {
    // If we are waiting for reads and the socket is not connect,
    // it means the connection was closed.
    if (is_reading_ || socket_->Connect(addresses_[0]) != net::OK) {
      setReadyState(READY_STATE_CLOSED);
      DispatchEvent("error");
      return;
    }
  }

  int ret = socket_->SendTo(
      write_buffer_,
      write_buffer_size_,
      addresses_[0],
      base::Bind(&UDPSocketObject::OnWrite, base::Unretained(this)));

  if (ret == net::ERR_IO_PENDING) {
    has_write_pending_ = true;
  } else if (ret == write_buffer_size_) {
    has_write_pending_ = false;
  } else {
    socket_->Close();
    setReadyState(READY_STATE_CLOSED);
    DispatchEvent("close");
    return;
  }

  if (!is_reading_ && socket_->is_connected())
    DoRead();
}

}  // namespace sysapps
}  // namespace xwalk
