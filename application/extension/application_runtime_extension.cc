// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/extension/application_runtime_extension.h"

#include "base/bind.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "ipc/ipc_message.h"
#include "grit/xwalk_application_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/runtime/browser/runtime.h"

using content::BrowserThread;

namespace xwalk {
namespace application {

ApplicationRuntimeExtension::ApplicationRuntimeExtension(
    Application* application)
  : application_(application) {
  set_name("xwalk.app.runtime");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_APPLICATION_RUNTIME_API).as_string());
}

XWalkExtensionInstance* ApplicationRuntimeExtension::CreateInstance() {
  return new AppRuntimeExtensionInstance(application_);
}

AppRuntimeExtensionInstance::AppRuntimeExtensionInstance(
    Application* application)
  : application_(application),
    handler_(this) {
  handler_.Register(
      "getManifest",
      base::Bind(&AppRuntimeExtensionInstance::OnGetManifest,
                 base::Unretained(this)));
}

void AppRuntimeExtensionInstance::HandleMessage(std::unique_ptr<base::Value> msg) {
  handler_.HandleMessage(std::move(msg));
}

void AppRuntimeExtensionInstance::OnGetManifest(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  base::DictionaryValue* manifest_data =
          application_->data()->GetManifest()->value()->DeepCopy();

  std::unique_ptr<base::ListValue> results(new base::ListValue());
  if (manifest_data)
    results->Append(manifest_data);
  else
    // Return an empty dictionary value when there's no valid manifest data.
    results->Append(new base::DictionaryValue());
  info->PostResult(std::move(results));
}

}  // namespace application
}  // namespace xwalk
