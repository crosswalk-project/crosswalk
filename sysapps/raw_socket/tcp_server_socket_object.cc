// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/raw_socket/tcp_server_socket_object.h"

#include <string.h>
#include "base/guid.h"
#include "base/logging.h"
#include "net/base/ip_endpoint.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "net/socket/stream_socket.h"
#include "xwalk/sysapps/common/binding_object_store.h"
#include "xwalk/sysapps/raw_socket/tcp_server_socket.h"
#include "xwalk/sysapps/raw_socket/tcp_socket.h"
#include "xwalk/sysapps/raw_socket/tcp_socket_object.h"

using namespace xwalk::jsapi::tcp_server_socket; // NOLINT
using namespace xwalk::jsapi::raw_socket; // NOLINT

namespace xwalk {
namespace sysapps {

TCPServerSocketObject::TCPServerSocketObject(RawSocketInstance* instance)
  : is_suspended_(false),
    is_accepting_(false),
    instance_(instance) {
  handler_.Register("init",
      base::Bind(&TCPServerSocketObject::OnInit, base::Unretained(this)));
  handler_.Register("_close",
      base::Bind(&TCPServerSocketObject::OnClose, base::Unretained(this)));
  handler_.Register("suspend",
      base::Bind(&TCPServerSocketObject::OnSuspend, base::Unretained(this)));
  handler_.Register("resume",
      base::Bind(&TCPServerSocketObject::OnResume, base::Unretained(this)));
}

TCPServerSocketObject::~TCPServerSocketObject() {}

void TCPServerSocketObject::DoAccept() {
  socket_->Accept(&accepted_socket_,
                  base::Bind(&TCPServerSocketObject::OnAccept,
                             base::Unretained(this)));
}

void TCPServerSocketObject::StartEvent(const std::string& type) {
  if (type == "connect")
    is_accepting_ = true;
}

void TCPServerSocketObject::StopEvent(const std::string& type) {
  if (type == "connect")
    is_accepting_ = false;
}

void TCPServerSocketObject::OnInit(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<Init::Params> params(Init::Params::Create(*info->arguments()));
  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    setReadyState(READY_STATE_CLOSED);
    DispatchEvent("error");
    return;
  }

  net::IPAddressNumber ip_number;
  if (!net::ParseIPLiteralToNumber(params->options.local_address, &ip_number)) {
    LOG(WARNING) << "Invalid IP address " << params->options.local_address;
    setReadyState(READY_STATE_CLOSED);
    DispatchEvent("error");
    return;
  }

  socket_.reset(new net::TCPServerSocket(NULL, net::NetLog::Source()));
  net::IPEndPoint address(ip_number, params->options.local_port);

  if (socket_->Listen(address, 5) != net::OK) {
    LOG(WARNING) << "Failed to listen on " << params->options.local_address
        << " port " << params->options.local_port;
    setReadyState(READY_STATE_CLOSED);
    DispatchEvent("error");
    return;
  }

  setReadyState(READY_STATE_OPEN);
  DispatchEvent("open");
  DoAccept();
}

void TCPServerSocketObject::OnClose(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  if (socket_)
    socket_.reset();

  setReadyState(READY_STATE_CLOSED);
  DispatchEvent("close");
}

void TCPServerSocketObject::OnSuspend(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  is_suspended_ = true;
}

void TCPServerSocketObject::OnResume(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  is_suspended_ = false;
}

void TCPServerSocketObject::OnAccept(int status) {
  if (is_accepting_ && !is_suspended_) {
    net::IPEndPoint local_address;
    accepted_socket_->GetLocalAddress(&local_address);

    jsapi::tcp_socket::TCPOptions options;
    options.local_address = local_address.ToStringWithoutPort();
    options.local_port = local_address.port();
    options.address_reuse = false;
    options.no_delay = true;
    options.use_secure_transport = false;

    std::string object_id = base::GenerateGUID();
    scoped_ptr<BindingObject> obj(new TCPSocketObject(accepted_socket_.Pass()));
    instance_->AddBindingObject(object_id, obj.Pass());

    scoped_ptr<base::ListValue> dataList(new base::ListValue);
    dataList->AppendString(object_id);
    dataList->Append(options.ToValue().release());

    scoped_ptr<base::ListValue> eventData(new base::ListValue);
    eventData->Append(dataList.release());

    DispatchEvent("connect", eventData.Pass());
  } else {
    // The spec is not really clear about what to do when we get a incoming
    // connection but nobody is listening. We are just closing the socket in
    // this case.
    accepted_socket_.reset();
  }

  DoAccept();
}

}  // namespace sysapps
}  // namespace xwalk
