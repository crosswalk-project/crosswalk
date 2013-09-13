// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/raw_socket/raw_socket_extension.h"

#include "xwalk/jsapi/sysapps_raw_socket.h"
#include "xwalk/sysapps/raw_socket/tcp_socket_object.h"

// This will be generated from common_api.js and raw_socket_api.js.
extern const char kSource_common_api[];
extern const char kSource_raw_socket_api[];

using namespace xwalk::jsapi::sysapps_raw_socket; // NOLINT

namespace xwalk {
namespace sysapps {

RawSocketExtension::RawSocketExtension()
    : XWalkInternalExtension() {
  set_name("xwalk.sysapps.raw_socket");

  // FIXME(tmpsantos): This is a workaround until we
  // get require("something") in place.
  api += kSource_common_api;
  api += kSource_raw_socket_api;
}

RawSocketExtension::~RawSocketExtension() {}

const char* RawSocketExtension::GetJavaScriptAPI() {
  return api.c_str();
}

XWalkExtensionInstance* RawSocketExtension::CreateInstance(
    const XWalkExtension::PostMessageCallback& post_message) {
  return new RawSocketInstance(post_message);
}

RawSocketInstance::RawSocketInstance(
    const XWalkExtension::PostMessageCallback& post_message)
    : BindingObjectStore(post_message) {
  RegisterFunction("TCPSocketConstructor",
                   &RawSocketInstance::OnTCPSocketConstructor);
}

void RawSocketInstance::OnTCPSocketConstructor(const FunctionInfo& info) {
  scoped_ptr<TCPSocketConstructor::Params>
      params(TCPSocketConstructor::Params::Create(*info.arguments));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info.name;
    return;
  }

  scoped_ptr<BindingObject> obj(new TCPSocketObject);
  obj->set_instance(this);

  AddBindingObject(params->object_id, obj.Pass());
}

}  // namespace sysapps
}  // namespace xwalk
