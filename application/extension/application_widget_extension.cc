// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/extension/application_widget_extension.h"

#include "base/bind.h"
#include "base/path_service.h"
#include "base/strings/string_util.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "ipc/ipc_message.h"
#include "grit/xwalk_application_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/widget_handler.h"
#include "xwalk/application/extension/application_widget_storage.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_paths.h"

using content::BrowserThread;

namespace {
const char kCommandKey[] = "cmd";
const char kWidgetAttributeKey[] = "widgetKey";
const char kPreferencesItemKey[] = "preferencesItemKey";
const char kPreferencesItemValue[] = "preferencesItemValue";
}

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
  : application_(application),
    handler_(this) {
  DCHECK(application_);
  base::ThreadRestrictions::SetIOAllowed(true);

  base::FilePath path;
  xwalk::RegisterPathProvider();
  PathService::Get(xwalk::DIR_WGT_STORAGE_PATH, &path);
  widget_storage_.reset(new AppWidgetStorage(application_, path));
}

AppWidgetExtensionInstance::~AppWidgetExtensionInstance() {}

void AppWidgetExtensionInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  handler_.HandleMessage(msg.Pass());
}

void AppWidgetExtensionInstance::HandleSyncMessage(
    scoped_ptr<base::Value> msg) {
  base::DictionaryValue* dict;
  std::string command;
  msg->GetAsDictionary(&dict);

  if (!msg->GetAsDictionary(&dict) || !dict->GetString(kCommandKey, &command)) {
    LOG(ERROR) << "Fail to handle command sync message.";
    SendSyncReplyToJS(scoped_ptr<base::Value>(
        base::Value::CreateStringValue("")));
    return;
  }

  scoped_ptr<base::Value> result(base::Value::CreateStringValue(""));
  if (command == "GetWidgetInfo") {
    result = GetWidgetInfo(msg.Pass());
  } else if (command == "SetPreferencesItem") {
    result = SetPreferencesItem(msg.Pass());
  } else if (command == "RemovePreferencesItem") {
    result = RemovePreferencesItem(msg.Pass());
  } else if (command == "ClearAllItems") {
    result = ClearAllItems(msg.Pass());
  } else if (command == "GetAllItems") {
    result = GetAllItems(msg.Pass());
  } else if (command == "KeyExists") {
    result = KeyExists(msg.Pass());
  } else {
    LOG(ERROR) << command << " ASSERT NOT REACHED.";
  }

  SendSyncReplyToJS(result.Pass());
}

scoped_ptr<base::StringValue> AppWidgetExtensionInstance::GetWidgetInfo(
    scoped_ptr<base::Value> msg) {
  scoped_ptr<base::StringValue> result(base::Value::CreateStringValue(""));
  std::string key;
  std::string value;
  base::DictionaryValue* dict;

  if (!msg->GetAsDictionary(&dict) ||
      !dict->GetString(kWidgetAttributeKey, &key)) {
    LOG(ERROR) << "Fail to get widget attribute key.";
    return result.Pass();
  }

  WidgetInfo* info =
      static_cast<WidgetInfo*>(
      application_->data()->GetManifestData(widget_keys::kWidgetKey));
  base::DictionaryValue* widget_info = info->GetWidgetInfo();
  widget_info->GetString(key, &value);
  result.reset(base::Value::CreateStringValue(value));
  return result.Pass();
}

scoped_ptr<base::FundamentalValue>
AppWidgetExtensionInstance::SetPreferencesItem(scoped_ptr<base::Value> msg) {
  scoped_ptr<base::FundamentalValue> result(
      base::Value::CreateBooleanValue(false));
  std::string key;
  std::string value;
  base::DictionaryValue* dict;

  if (!msg->GetAsDictionary(&dict) ||
      !dict->GetString(kPreferencesItemKey, &key) ||
      !dict->GetString(kPreferencesItemValue, &value)) {
    LOG(ERROR) << "Fail to set preferences item.";
    return result.Pass();
  }

  if (widget_storage_->AddEntry(key, value, false))
    result.reset(base::Value::CreateBooleanValue(true));

  return result.Pass();
}

scoped_ptr<base::FundamentalValue>
AppWidgetExtensionInstance::RemovePreferencesItem(scoped_ptr<base::Value> msg) {
  scoped_ptr<base::FundamentalValue> result(
      base::Value::CreateBooleanValue(false));
  std::string key;
  base::DictionaryValue* dict;

  if (!msg->GetAsDictionary(&dict) ||
      !dict->GetString(kPreferencesItemKey, &key)) {
    LOG(ERROR) << "Fail to remove preferences item.";
    return result.Pass();
  }

  if (widget_storage_->RemoveEntry(key))
    result.reset(base::Value::CreateBooleanValue(true));

  return result.Pass();
}

scoped_ptr<base::FundamentalValue> AppWidgetExtensionInstance::ClearAllItems(
    scoped_ptr<base::Value> msg) {
  scoped_ptr<base::FundamentalValue> result(
      base::Value::CreateBooleanValue(false));

  if (widget_storage_->Clear())
    result.reset(base::Value::CreateBooleanValue(true));

  return result.Pass();
}

scoped_ptr<base::DictionaryValue> AppWidgetExtensionInstance::GetAllItems(
    scoped_ptr<base::Value> msg) {
  scoped_ptr<base::DictionaryValue> result(new base::DictionaryValue());
  widget_storage_->GetAllEntries(result.get());

  return result.Pass();
}

scoped_ptr<base::FundamentalValue> AppWidgetExtensionInstance::KeyExists(
    scoped_ptr<base::Value> msg) const {
  scoped_ptr<base::FundamentalValue> result(
      base::Value::CreateBooleanValue(false));
  std::string key;
  base::DictionaryValue* dict;

  if (!msg->GetAsDictionary(&dict) ||
      !dict->GetString(kPreferencesItemKey, &key)) {
    LOG(ERROR) << "Fail to remove preferences item.";
    return result.Pass();
  }

  if (widget_storage_->EntryExists(key))
    result.reset(base::Value::CreateBooleanValue(true));

  return result.Pass();
}

}  // namespace application
}  // namespace xwalk
