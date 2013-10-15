// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/raw_socket/raw_socket_extension.h"

#include "grit/xwalk_sysapps_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/sysapps/raw_socket/raw_socket.h"

using namespace xwalk::jsapi::raw_socket; // NOLINT

namespace xwalk {
namespace sysapps {

RawSocketExtension::RawSocketExtension() {
  set_name("xwalk.sysapps.raw_socket");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_SYSAPPS_RAW_SOCKET_API).as_string());
}

RawSocketExtension::~RawSocketExtension() {}

XWalkExtensionInstance* RawSocketExtension::CreateInstance() {
  return new RawSocketInstance();
}

RawSocketInstance::RawSocketInstance() {
}

}  // namespace sysapps
}  // namespace xwalk
