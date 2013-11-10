// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/extension/runtime_extension.h"

#include "base/bind.h"
#include "grit/xwalk_resources.h"
#include "xwalk/runtime/extension/runtime.h"
#include "ui/base/resource/resource_bundle.h"

namespace xwalk {

RuntimeExtension::RuntimeExtension() {
  set_name("xwalk.runtime");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_RUNTIME_API).as_string());
}

XWalkExtensionInstance* RuntimeExtension::CreateInstance() {
  return new RuntimeInstance();
}

RuntimeInstance::RuntimeInstance() : handler_(this) {
  handler_.Register("getAPIVersion",
      base::Bind(&RuntimeInstance::OnGetAPIVersion, base::Unretained(this)));
}

void RuntimeInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  handler_.HandleMessage(msg.Pass());
}

void RuntimeInstance::OnGetAPIVersion(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  info->PostResult(jsapi::runtime::GetAPIVersion::Results::Create(1));
};

}  // namespace xwalk
