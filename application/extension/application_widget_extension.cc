// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/extension/application_widget_extension.h"

#include "base/bind.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "ipc/ipc_message.h"
#include "grit/xwalk_application_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/widget_handler.h"
#include "xwalk/runtime/browser/runtime.h"

using content::BrowserThread;

namespace xwalk {
namespace application {

namespace widget_keys = xwalk::application_widget_keys;

ApplicationWidgetExtension::ApplicationWidgetExtension(
    Application* application)
  : application_(application) {
  set_name("widget");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_APPLICATION_WIDGET_API).as_string());
}

XWalkExtensionInstance* ApplicationWidgetExtension::CreateInstance() {
  return new AppWidgetExtensionInstance(application_);
}

AppWidgetExtensionInstance::AppWidgetExtensionInstance(
    Application* application)
  : application_(application) {
  DCHECK(application_);
}

void AppWidgetExtensionInstance::HandleMessage(scoped_ptr<base::Value> msg) {
}

void AppWidgetExtensionInstance::HandleSyncMessage(
    scoped_ptr<base::Value> msg) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  std::string key;
  base::Value* result;
  base::StringValue* ret_val = base::Value::CreateStringValue("");

  if (!msg->GetAsString(&key)) {
    LOG(ERROR) << "Fail to get sync message as manifest widget key.";
    SendSyncReplyToJS(scoped_ptr<base::Value>(ret_val));
    return;
  }

  WidgetInfo* info =
      static_cast<WidgetInfo*>(
      application_->data()->GetManifestData(widget_keys::kWidgetKey));
  base::DictionaryValue* value = info->GetWidgetInfo();
  if (!value->Get(key, &result)) {
    LOG(ERROR) << "Fail to get value for key: " << key;
    SendSyncReplyToJS(scoped_ptr<base::Value>(ret_val));
  } else {
    SendSyncReplyToJS(scoped_ptr<base::Value>(result));
  }
}

}  // namespace application
}  // namespace xwalk
