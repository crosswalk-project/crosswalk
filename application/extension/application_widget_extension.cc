// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/extension/application_widget_extension.h"

#include <vector>

#include "base/bind.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/storage_partition.h"
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
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_paths.h"

using content::BrowserThread;

namespace {

const char kCommandKey[] = "cmd";
const char kWidgetAttributeKey[] = "widgetKey";
const char kPreferencesItemKey[] = "preferencesItemKey";
const char kPreferencesItemValue[] = "preferencesItemValue";

void DispatchStorageEvent(
    base::DictionaryValue* msg,
    content::WebContents* web_contents,
    content::RenderFrameHost* frame) {
  if (frame == web_contents->GetFocusedFrame())
    return;

  std::string key, oldValue, newValue;
  msg->GetString("key", &key);
  msg->GetString("oldValue", &oldValue);
  msg->GetString("newValue", &newValue);

  std::string code = base::StringPrintf(
      "(function() {"
      "  var old_value = '%s' == '' ? null : '%s';"
      "  var new_value = '%s' == '' ? null : '%s';"
      "  var event = {"
      "    key: '%s',"
      "    oldValue: old_value,"
      "    newValue: new_value,"
      "    url: window.location.href,"
      "    storageArea: widget.preferences"
      "  };"
      "  for (var key in event) {"
      "    Object.defineProperty(event, key, {"
      "      value: event[key],"
      "      writable: false"
      "    });"
      "  }"
      "  for (var i = 0; i < window.eventListenerList.length; i++)"
      "    window.eventListenerList[i](event);"
      "})();", oldValue.c_str(), oldValue.c_str(),
      newValue.c_str(), newValue.c_str(), key.c_str());

  frame->ExecuteJavaScript(base::UTF8ToUTF16(code));
}

}  // namespace

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
  base::ThreadRestrictions::SetIOAllowed(true);

  content::RenderProcessHost* rph =
      content::RenderProcessHost::FromID(application->GetRenderProcessHostID());
  CHECK(rph);
  content::StoragePartition* partition = rph->GetStoragePartition();
  CHECK(partition);
  base::FilePath path = partition->GetPath().Append(
      FILE_PATH_LITERAL("WidgetStorage"));
  widget_storage_.reset(new AppWidgetStorage(application_, path));
}

AppWidgetExtensionInstance::~AppWidgetExtensionInstance() {}

void AppWidgetExtensionInstance::HandleMessage(scoped_ptr<base::Value> msg) {
}

void AppWidgetExtensionInstance::HandleSyncMessage(
    scoped_ptr<base::Value> msg) {
  base::DictionaryValue* dict;
  std::string command;
  msg->GetAsDictionary(&dict);

  if (!msg->GetAsDictionary(&dict) || !dict->GetString(kCommandKey, &command)) {
    LOG(ERROR) << "Fail to handle command sync message.";
    SendSyncReplyToJS(scoped_ptr<base::Value>(new base::StringValue("")));
    return;
  }

  scoped_ptr<base::Value> result(new base::StringValue(""));
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
  } else if (command == "GetItemValueByKey") {
    result = GetItemValueByKey(msg.Pass());
  } else if (command == "KeyExists") {
    result = KeyExists(msg.Pass());
  } else {
    LOG(ERROR) << command << " ASSERT NOT REACHED.";
  }

  SendSyncReplyToJS(result.Pass());
}

scoped_ptr<base::StringValue> AppWidgetExtensionInstance::GetWidgetInfo(
    scoped_ptr<base::Value> msg) {
  scoped_ptr<base::StringValue> result(new base::StringValue(""));
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
  result.reset(new base::StringValue(value));
  return result.Pass();
}

scoped_ptr<base::FundamentalValue>
AppWidgetExtensionInstance::SetPreferencesItem(scoped_ptr<base::Value> msg) {
  scoped_ptr<base::FundamentalValue> result(
      new base::FundamentalValue(false));
  std::string key;
  std::string value;
  base::DictionaryValue* dict;

  if (!msg->GetAsDictionary(&dict) ||
      !dict->GetString(kPreferencesItemKey, &key) ||
      !dict->GetString(kPreferencesItemValue, &value)) {
    LOG(ERROR) << "Fail to set preferences item.";
    return result.Pass();
  }

  std::string old_value;
  if (!widget_storage_->GetValueByKey(key, &old_value)) {
    old_value = "";
  }
  if (old_value == value) {
    LOG(WARNING) << "You are trying to set the same value."
                 << " Nothing will be done.";
    result.reset(new base::FundamentalValue(true));
    return result.Pass();
  }
  if (widget_storage_->AddEntry(key, value, false)) {
    result.reset(new base::FundamentalValue(true));

    scoped_ptr<base::DictionaryValue> event(new base::DictionaryValue());
    event->SetString("key", key);
    event->SetString("oldValue", old_value);
    event->SetString("newValue", value);
    PostMessageToOtherFrames(event.Pass());
  }

  return result.Pass();
}

scoped_ptr<base::FundamentalValue>
AppWidgetExtensionInstance::RemovePreferencesItem(scoped_ptr<base::Value> msg) {
  scoped_ptr<base::FundamentalValue> result(
      new base::FundamentalValue(false));
  std::string key;
  base::DictionaryValue* dict;

  if (!msg->GetAsDictionary(&dict) ||
      !dict->GetString(kPreferencesItemKey, &key)) {
    LOG(ERROR) << "Fail to remove preferences item.";
    return result.Pass();
  }

  std::string old_value;
  if (!widget_storage_->GetValueByKey(key, &old_value)) {
    LOG(WARNING) << "You are trying to remove an entry which doesn't exist."
                 << " Nothing will be done.";
    result.reset(new base::FundamentalValue(true));
    return result.Pass();
  }

  if (widget_storage_->RemoveEntry(key)) {
    result.reset(new base::FundamentalValue(true));

    scoped_ptr<base::DictionaryValue> event(new base::DictionaryValue());
    event->SetString("key", key);
    event->SetString("oldValue", old_value);
    event->SetString("newValue", "");
    PostMessageToOtherFrames(event.Pass());
  }

  return result.Pass();
}

scoped_ptr<base::FundamentalValue> AppWidgetExtensionInstance::ClearAllItems(
    scoped_ptr<base::Value> msg) {
  scoped_ptr<base::FundamentalValue> result(
      new base::FundamentalValue(false));

  scoped_ptr<base::DictionaryValue> entries(new base::DictionaryValue());
  widget_storage_->GetAllEntries(entries.get());

  if (!widget_storage_->Clear())
    return result.Pass();

  for (base::DictionaryValue::Iterator it(*(entries.get()));
      !it.IsAtEnd(); it.Advance()) {
    std::string key = it.key();
    if (!widget_storage_->EntryExists(key)) {
      std::string old_value;
      it.value().GetAsString(&old_value);
      scoped_ptr<base::DictionaryValue> event(new base::DictionaryValue());
      event->SetString("key", key);
      event->SetString("oldValue", old_value);
      event->SetString("newValue", "");
      PostMessageToOtherFrames(event.Pass());
    }
  }

  result.reset(new base::FundamentalValue(true));
  return result.Pass();
}

scoped_ptr<base::DictionaryValue> AppWidgetExtensionInstance::GetAllItems(
    scoped_ptr<base::Value> msg) {
  scoped_ptr<base::DictionaryValue> result(new base::DictionaryValue());
  widget_storage_->GetAllEntries(result.get());

  return result.Pass();
}

scoped_ptr<base::StringValue> AppWidgetExtensionInstance::GetItemValueByKey(
    scoped_ptr<base::Value> msg) {
  base::DictionaryValue* dict;
  msg->GetAsDictionary(&dict);

  std::string key;
  std::string value;
  if (!dict->GetString(kPreferencesItemKey, &key) ||
      !widget_storage_->GetValueByKey(key, &value))
    value = "";
  scoped_ptr<base::StringValue> result(new base::StringValue(value));
  return result.Pass();
}

scoped_ptr<base::FundamentalValue> AppWidgetExtensionInstance::KeyExists(
    scoped_ptr<base::Value> msg) const {
  scoped_ptr<base::FundamentalValue> result(
      new base::FundamentalValue(false));
  std::string key;
  base::DictionaryValue* dict;

  if (!msg->GetAsDictionary(&dict) ||
      !dict->GetString(kPreferencesItemKey, &key)) {
    LOG(ERROR) << "Fail to remove preferences item.";
    return result.Pass();
  }

  if (widget_storage_->EntryExists(key))
    result.reset(new base::FundamentalValue(true));

  return result.Pass();
}

void AppWidgetExtensionInstance::PostMessageToOtherFrames(
    scoped_ptr<base::DictionaryValue> msg) {
  const std::vector<Runtime*>& runtime_set = application_->runtimes();
  std::vector<Runtime*>::const_iterator it;
  for (it = runtime_set.begin(); it != runtime_set.end(); ++it) {
    content::WebContents* web_contents = (*it)->web_contents();
    web_contents->ForEachFrame(base::Bind(&DispatchStorageEvent,
                                          msg.get(),
                                          web_contents));
  }
}

}  // namespace application
}  // namespace xwalk
