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

XWalkExtensionInstance* RuntimeExtension::CreateInstance() {
  return new RuntimeInstance();
}

RuntimeInstance::RuntimeInstance()
  : XWalkInternalExtensionInstance() {
  handler_.Register("getAPIVersion",
      base::Bind(&RuntimeInstance::OnGetAPIVersion, base::Unretained(this)));
}

void RuntimeInstance::OnGetAPIVersion(const XWalkExtensionFunctionInfo& info) {
  PostResult(info.callback_id,
             jsapi::runtime::GetAPIVersion::Results::Create(1));
};

}  // namespace xwalk
