// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/extension/application_event_extension.h"

#include <set>

#include "base/stl_util.h"
#include "content/public/browser/web_contents.h"
#include "grit/xwalk_application_resources.h"
#include "ipc/ipc_message.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application_storage.h"
#include "xwalk/application/common/event_names.h"
#include "xwalk/runtime/browser/runtime.h"

namespace xwalk {

using application::ApplicationData;
using application::Event;

ApplicationEventExtension::ApplicationEventExtension(
    application::ApplicationEventManager* event_manager,
    application::ApplicationStorage* app_storage,
    application::Application* app)
  : event_manager_(event_manager),
    app_storage_(app_storage),
    application_(app) {
  set_name("xwalk.app.events");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_APPLICATION_EVENT_API).as_string());
}

XWalkExtensionInstance* ApplicationEventExtension::CreateInstance() {
  int main_routing_id = MSG_ROUTING_NONE;

  if (Runtime* runtime = application_->GetMainDocumentRuntime())
    main_routing_id = runtime->web_contents()->GetRoutingID();

  return new AppEventExtensionInstance(event_manager_, app_storage_,
                                       application_, main_routing_id);
}

AppEventExtensionInstance::AppEventExtensionInstance(
    application::ApplicationEventManager* event_manager,
    application::ApplicationStorage* app_storage,
    application::Application* app,
    int main_routing_id)
  : application::EventObserver(event_manager),
    app_storage_(app_storage),
    application_(app),
    main_routing_id_(main_routing_id),
    handler_(this) {
  handler_.Register("registerEvent",
                    base::Bind(&AppEventExtensionInstance::OnRegisterEvent,
                    base::Unretained(this)));
  handler_.Register("unregisterEvent",
                    base::Bind(&AppEventExtensionInstance::OnUnregisterEvent,
                    base::Unretained(this)));
  handler_.Register("dispatchEventFinish",
                    base::Bind(
                        &AppEventExtensionInstance::OnDispatchEventFinish,
                        base::Unretained(this)));
}

AppEventExtensionInstance::~AppEventExtensionInstance() {
}

void AppEventExtensionInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  handler_.HandleMessage(msg.Pass());
}

void AppEventExtensionInstance::OnRegisterEvent(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  std::string event_name;
  int routing_id;
  if (info->arguments()->GetSize() != 2 ||
      !info->arguments()->GetString(0, &event_name) ||
      !info->arguments()->GetInteger(1, &routing_id))
    return;

  if (!ContainsKey(registered_events_, event_name)) {
    registered_events_.insert(
        std::make_pair(event_name, info->post_result_cb()));
    event_manager_->AttachObserver(application_->id(), event_name, this);

    if (routing_id != main_routing_id_)
      return;

    // If the event is from main document, add it into system database.
    scoped_refptr<application::ApplicationData> app_data =
        app_storage_->GetApplicationData(application_->id());
    if (!app_data)
      return;

    std::set<std::string> events = app_data->GetEvents();
    if (ContainsKey(events, event_name))
      return;
    events.insert(event_name);
    app_data->SetEvents(events);
    app_storage_->UpdateApplication(app_data);
  }
}

void AppEventExtensionInstance::OnUnregisterEvent(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  std::string event_name;
  int routing_id;
  if (info->arguments()->GetSize() != 2 ||
      !info->arguments()->GetString(0, &event_name) ||
      !info->arguments()->GetInteger(1, &routing_id))
    return;

  if (!ContainsKey(registered_events_, event_name))
    return;

  registered_events_.erase(event_name);
  event_manager_->DetachObserver(application_->id(), event_name, this);

  // If the event is from main document, remove it from system database.
  if (routing_id == main_routing_id_) {
    scoped_refptr<application::ApplicationData> app_data =
        app_storage_->GetApplicationData(application_->id());
    if (!app_data)
      return;

    std::set<std::string> events = app_data->GetEvents();
    if (!ContainsKey(events, event_name))
      return;
    events.erase(event_name);
    app_data->SetEvents(events);
    app_storage_->UpdateApplication(app_data);
  }
}

void AppEventExtensionInstance::OnDispatchEventFinish(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  std::string event_name;
  if (info->arguments()->GetSize() != 1 ||
      !info->arguments()->GetString(0, &event_name))
    return;

  scoped_ptr<base::ListValue> args(new base::ListValue());
  args->AppendString(event_name);
  scoped_refptr<Event> event = Event::CreateEvent(
      application::kOnJavaScriptEventAck, args.Pass());
  event_manager_->SendEvent(application_->id(), event);
}

void AppEventExtensionInstance::Observe(
    const std::string& app_id, scoped_refptr<application::Event> event) {
  DCHECK(application_->id() == app_id);
  EventCallbackMap::iterator it = registered_events_.find(event->name());
  if (it == registered_events_.end())
    return;

  scoped_ptr<base::ListValue> args(new base::ListValue());
  args->Append(event->args()->DeepCopy());
  it->second.Run(args.Pass());
}

}  // namespace xwalk
