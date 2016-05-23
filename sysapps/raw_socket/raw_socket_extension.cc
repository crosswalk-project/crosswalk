// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/raw_socket/raw_socket_extension.h"

#include "grit/xwalk_sysapps_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/sysapps/raw_socket/raw_socket.h"
#include "xwalk/sysapps/raw_socket/tcp_server_socket_object.h"
#include "xwalk/sysapps/raw_socket/tcp_socket_object.h"
#include "xwalk/sysapps/raw_socket/udp_socket_object.h"

using namespace xwalk::jsapi::raw_socket; // NOLINT

namespace xwalk {
namespace sysapps {

RawSocketExtension::RawSocketExtension() {
  set_name("xwalk.experimental.raw_socket");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_SYSAPPS_RAW_SOCKET_API).as_string());
}

RawSocketExtension::~RawSocketExtension() {}

XWalkExtensionInstance* RawSocketExtension::CreateInstance() {
  return new RawSocketInstance();
}

RawSocketInstance::RawSocketInstance()
  : handler_(this),
    store_(&handler_) {
  handler_.Register("TCPServerSocketConstructor",
      base::Bind(&RawSocketInstance::OnTCPServerSocketConstructor,
                 base::Unretained(this)));
  handler_.Register("TCPSocketConstructor",
      base::Bind(&RawSocketInstance::OnTCPSocketConstructor,
                 base::Unretained(this)));
  handler_.Register("UDPSocketConstructor",
      base::Bind(&RawSocketInstance::OnUDPSocketConstructor,
                 base::Unretained(this)));
}

void RawSocketInstance::HandleMessage(std::unique_ptr<base::Value> msg) {
  handler_.HandleMessage(std::move(msg));
}

void RawSocketInstance::AddBindingObject(const std::string& object_id,
                                         std::unique_ptr<BindingObject> obj) {
  store_.AddBindingObject(object_id, std::move(obj));
}

void RawSocketInstance::OnTCPServerSocketConstructor(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::unique_ptr<TCPServerSocketConstructor::Params>
      params(TCPServerSocketConstructor::Params::Create(*info->arguments()));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }

  std::unique_ptr<BindingObject> obj(new TCPServerSocketObject(this));
  store_.AddBindingObject(params->object_id, std::move(obj));
}

void RawSocketInstance::OnTCPSocketConstructor(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::unique_ptr<TCPSocketConstructor::Params>
      params(TCPSocketConstructor::Params::Create(*info->arguments()));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }

  std::unique_ptr<BindingObject> obj(new TCPSocketObject);
  store_.AddBindingObject(params->object_id, std::move(obj));
}

void RawSocketInstance::OnUDPSocketConstructor(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::unique_ptr<UDPSocketConstructor::Params>
      params(UDPSocketConstructor::Params::Create(*info->arguments()));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }

  std::unique_ptr<BindingObject> obj(new UDPSocketObject);
  store_.AddBindingObject(params->object_id, std::move(obj));
}

}  // namespace sysapps
}  // namespace xwalk
