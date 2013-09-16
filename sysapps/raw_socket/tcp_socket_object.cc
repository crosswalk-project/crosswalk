// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/raw_socket/tcp_socket_object.h"

#include <string.h>
#include "base/logging.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "xwalk/jsapi/sysapps_raw_socket_tcp_socket.h"
#include "xwalk/sysapps/raw_socket/raw_socket_extension.h"

using namespace xwalk::jsapi::sysapps_raw_socket_tcp_socket; // NOLINT

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
      single_resolver_(resolver_.get()) {
  RegisterFunction("init", &TCPSocketObject::OnInit);
  RegisterFunction("_close", &TCPSocketObject::OnClose);
  RegisterFunction("halfclose", &TCPSocketObject::OnHalfClose);
  RegisterFunction("suspend", &TCPSocketObject::OnSuspend);
  RegisterFunction("resume", &TCPSocketObject::OnResume);
  RegisterFunction("_sendString", &TCPSocketObject::OnSendString);
}

TCPSocketObject::~TCPSocketObject() {}

void TCPSocketObject::setReadyState(ReadyState state) {
  scoped_ptr<base::ListValue> eventData(new base::ListValue);
  eventData->AppendString(ToString(state));

  DispatchEvent("readystate", eventData.Pass());
}

void TCPSocketObject::DoRead() {
  if (!socket_->IsConnected())
    return;

  socket_->Read(read_buffer_,
                kBufferSize,
                base::Bind(&TCPSocketObject::OnRead,
                           base::Unretained(this)));
}

void TCPSocketObject::OnInit(const FunctionInfo& info) {
  scoped_ptr<Init::Params> params(Init::Params::Create(*info.arguments));
  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info.name;
    setReadyState(READY_STATE_CLOSED);
    DispatchEvent("error");
    return;
  }

  net::HostResolver::RequestInfo request_info(
      net::HostPortPair(params->remote_address, params->remote_port));

  int result = single_resolver_.Resolve(request_info, &addresses_,
      base::Bind(&TCPSocketObject::OnResolved,
                 base::Unretained(this)),
                 net::BoundNetLog());

  if (result != net::ERR_IO_PENDING)
    OnResolved(result);
}

void TCPSocketObject::OnClose(const FunctionInfo& info) {
  if (socket_)
    socket_->Disconnect();

  setReadyState(READY_STATE_CLOSED);
  DispatchEvent("close");
}

void TCPSocketObject::OnHalfClose(const FunctionInfo& info) {
  is_half_closed_ = true;

  if (socket_ && socket_->IsConnected())
    setReadyState(READY_STATE_HALFCLOSED);
}

void TCPSocketObject::OnSuspend(const FunctionInfo& info) {
  is_suspended_ = true;
}

void TCPSocketObject::OnResume(const FunctionInfo& info) {
  is_suspended_ = false;
}

void TCPSocketObject::OnSendString(const FunctionInfo& info) {
  if (is_half_closed_ || has_write_pending_)
    return;

  if (!socket_ || !socket_->IsConnected())
    return;

  scoped_ptr<SendDOMString::Params>
      params(SendDOMString::Params::Create(*info.arguments));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info.name;
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
      static_cast<char *>(read_buffer_->data()), status));

  eventData->Append(data.release());

  if (!is_suspended_)
    DispatchEvent("data", eventData.Pass());

  DoRead();
}

void TCPSocketObject::OnWrite(int status) {
  has_write_pending_ = false;
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
