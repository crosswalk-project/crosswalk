// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/extension/runtime_extension.h"

#include "base/bind.h"
#include "xwalk/jsapi/runtime.h"

extern const char kSource_runtime_api[];

namespace xwalk {

RuntimeExtension::RuntimeExtension() {
  set_name("xwalk.runtime");
}

const char* RuntimeExtension::GetJavaScriptAPI() {
  return kSource_runtime_api;
}

XWalkExtension::Context* RuntimeExtension::CreateContext(
    const XWalkExtension::PostMessageCallback& post_message) {
  return new RuntimeContext(post_message);
}

RuntimeExtension::RuntimeContext::RuntimeContext(
    const XWalkExtension::PostMessageCallback& post_message)
  : XWalkInternalExtension::InternalContext(post_message) {
  RegisterFunction("getAPIVersion", &RuntimeContext::OnGetAPIVersion);
}

void RuntimeExtension::RuntimeContext::OnGetAPIVersion(
    const std::string&, const std::string& callback_id,
    base::ListValue* args) {
  PostResult(callback_id, jsapi::runtime::GetAPIVersion::Results::Create(1));
};

}  // namespace xwalk
