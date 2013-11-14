// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/raw_socket/tcp_socket_object.h"

#include <string.h>
#include "base/logging.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "xwalk/sysapps/raw_socket/tcp_socket.h"

using namespace xwalk::jsapi::tcp_socket; // NOLINT
using namespace xwalk::jsapi::raw_socket; // NOLINT

namespace {

const unsigned kBufferSize = 4096;

}  // namespace

namespace xwalk {
namespace sysapps {

TCPSocketObject::TCPSocketObject()
    : has_write_pending_(false),
      is_suspended_(false),
      is_half_closed_(false),
      read_buffer_(new net::IOBuffer(kBufferSize)),
      write_buffer_(new net::IOBuffer(kBufferSize)),
      resolver_(net::HostResolver::CreateDefaultResolver(NULL)),
      single_resolver_(new net::SingleRequestHostResolver(resolver_.get())) {
  RegisterHandlers();
}

TCPSocketObject::TCPSocketObject(scoped_ptr<net::StreamSocket> socket)
    : has_write_pending_(false),
      is_suspended_(false),
      is_half_closed_(false),
      read_buffer_(new net::IOBuffer(kBufferSize)),
      write_buffer_(new net::IOBuffer(kBufferSize)),
      socket_(socket.release()) {
  RegisterHandlers();
}

TCPSocketObject::~TCPSocketObject() {}

void TCPSocketObject::RegisterHandlers() {
  handler_.Register("init",
      base::Bind(&TCPSocketObject::OnInit, base::Unretained(this)));
  handler_.Register("_close",
      base::Bind(&TCPSocketObject::OnClose, base::Unretained(this)));
  handler_.Register("halfclose",
      base::Bind(&TCPSocketObject::OnHalfClose, base::Unretained(this)));
  handler_.Register("suspend",
      base::Bind(&TCPSocketObject::OnSuspend, base::Unretained(this)));
  handler_.Register("resume",
      base::Bind(&TCPSocketObject::OnResume, base::Unretained(this)));
  handler_.Register("_sendString",
      base::Bind(&TCPSocketObject::OnSendString, base::Unretained(this)));
}

void TCPSocketObject::DoRead() {
  if (!socket_->IsConnected())
    return;

  int ret = socket_->Read(read_buffer_,
                          kBufferSize,
                          base::Bind(&TCPSocketObject::OnRead,
                                     base::Unretained(this)));

  if (ret > 0)
    OnRead(ret);
}

void TCPSocketObject::OnInit(scoped_ptr<XWalkExtensionFunctionInfo> info) {
  if (socket_) {
    DoRead();
    return;
  }

  scoped_ptr<Init::Params> params(Init::Params::Create(*info->arguments()));
  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    setReadyState(READY_STATE_CLOSED);
    DispatchEvent("error");
    return;
  }

  net::HostResolver::RequestInfo request_info(
      net::HostPortPair(params->remote_address, params->remote_port));

  int ret = single_resolver_->Resolve(
      request_info, net::DEFAULT_PRIORITY, &addresses_,
      base::Bind(&TCPSocketObject::OnResolved,
                 base::Unretained(this)),
                 net::BoundNetLog());

  if (ret != net::ERR_IO_PENDING)
    OnResolved(ret);
}

void TCPSocketObject::OnClose(scoped_ptr<XWalkExtensionFunctionInfo> info) {
  if (socket_)
    socket_->Disconnect();

  setReadyState(READY_STATE_CLOSED);
  DispatchEvent("close");
}

void TCPSocketObject::OnHalfClose(scoped_ptr<XWalkExtensionFunctionInfo> info) {
  if (!socket_ || !socket_->IsConnected())
    return;

  is_half_closed_ = true;
  setReadyState(READY_STATE_HALFCLOSED);
}

void TCPSocketObject::OnSuspend(scoped_ptr<XWalkExtensionFunctionInfo> info) {
  is_suspended_ = true;
}

void TCPSocketObject::OnResume(scoped_ptr<XWalkExtensionFunctionInfo> info) {
  is_suspended_ = false;
}

void TCPSocketObject::OnSendString(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  if (is_half_closed_ || has_write_pending_)
    return;

  if (!socket_ || !socket_->IsConnected())
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

  memcpy(write_buffer_->data(), params->data.data(), params->data.size());

  int ret = socket_->Write(write_buffer_,
                           params->data.size(),
                           base::Bind(&TCPSocketObject::OnWrite,
                                      base::Unretained(this)));

  if (ret == net::ERR_IO_PENDING)
    has_write_pending_ = true;
  else if (ret == static_cast<int>(params->data.size()))
    return;
  else
    socket_->Disconnect();
}

void TCPSocketObject::OnConnect(int status) {
  if (status == net::OK) {
    if (is_half_closed_)
      setReadyState(READY_STATE_HALFCLOSED);
    else
      setReadyState(READY_STATE_OPEN);

    DispatchEvent("open");
    DoRead();
  } else {
    setReadyState(READY_STATE_CLOSED);
    DispatchEvent("error");
  }
}

void TCPSocketObject::OnRead(int status) {
  scoped_ptr<base::ListValue> eventData(new base::ListValue);

  // No data means the other side has
  // disconnected the socket.
  if (status == 0) {
    setReadyState(READY_STATE_CLOSED);
    DispatchEvent("close", eventData.Pass());
    return;
  }

  scoped_ptr<base::Value> data(base::BinaryValue::CreateWithCopiedBuffer(
      static_cast<char*>(read_buffer_->data()), status));

  eventData->Append(data.release());

  if (!is_suspended_)
    DispatchEvent("data", eventData.Pass());

  DoRead();
}

void TCPSocketObject::OnWrite(int status) {
  has_write_pending_ = false;
  DispatchEvent("drain");
}

void TCPSocketObject::OnResolved(int status) {
  if (status != net::OK) {
    setReadyState(READY_STATE_CLOSED);
    DispatchEvent("error");
    return;
  }

  socket_.reset(new net::TCPClientSocket(addresses_,
                                         NULL,
                                         net::NetLog::Source()));

  socket_->Connect(base::Bind(&TCPSocketObject::OnConnect,
                              base::Unretained(this)));
}

}  // namespace sysapps
}  // namespace xwalk
