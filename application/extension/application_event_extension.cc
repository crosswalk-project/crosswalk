// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/extension/application_event_extension.h"

#include <set>

#include "base/stl_util.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/browser/web_contents.h"
#include "grit/xwalk_application_resources.h"
#include "ipc/ipc_message.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_storage.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/event_names.h"
#include "xwalk/runtime/browser/runtime.h"

namespace xwalk {
namespace application {

ApplicationEventExtension::ApplicationEventExtension(
    ApplicationSystem* system)
  : application_system_(system) {
  set_name("xwalk.app.events");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_APPLICATION_EVENT_API).as_string());
}

XWalkExtensionInstance* ApplicationEventExtension::CreateInstance() {
  ApplicationService* service = application_system_->application_service();

  int main_routing_id = MSG_ROUTING_NONE;
  // FIXME: return corresponding application info after shared runtime process
  // model is enabled.
  const Application* app = service->GetActiveApplication();
  CHECK(app);

  if (Runtime* runtime = app->GetMainDocumentRuntime())
    main_routing_id = runtime->web_contents()->GetRoutingID();

  return new AppEventExtensionInstance(
      application_system_,
      app->id(), main_routing_id);
}

AppEventExtensionInstance::AppEventExtensionInstance(
    ApplicationSystem* system,
    const std::string& app_id,
    int main_routing_id)
  : EventObserver(system ? system->event_manager(): NULL),
    app_system_(system),
    app_id_(app_id),
    main_routing_id_(main_routing_id),
    handler_(this) {
  DCHECK(app_system_);
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
  // FIXME: give UI thread I/O permission temporarily. The application storage
  // database should be accessed on DB thread.
  base::ThreadRestrictions::SetIOAllowed(true);
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
    event_manager_->AttachObserver(app_id_, event_name, this);

    if (routing_id != main_routing_id_)
      return;

    // If the event is from main document, add it into system database.
    application::ApplicationStorage* app_storage =
        app_system_->application_storage();
    scoped_refptr<application::ApplicationData> app_data =
        app_storage->GetApplicationData(app_id_);
    if (!app_data)
      return;

    std::set<std::string> events = app_data->GetEvents();
    if (ContainsKey(events, event_name))
      return;
    events.insert(event_name);
    app_data->SetEvents(events);
    app_storage->UpdateApplication(app_data);
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
  event_manager_->DetachObserver(app_id_, event_name, this);

  // If the event is from main document, remove it from system database.
  if (routing_id == main_routing_id_) {
    application::ApplicationStorage* app_storage =
        app_system_->application_storage();
    scoped_refptr<application::ApplicationData> app_data =
        app_storage->GetApplicationData(app_id_);
    if (!app_data)
      return;

    std::set<std::string> events = app_data->GetEvents();
    if (!ContainsKey(events, event_name))
      return;
    events.erase(event_name);
    app_data->SetEvents(events);
    app_storage->UpdateApplication(app_data);
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
      kOnJavaScriptEventAck, args.Pass());
  event_manager_->SendEvent(app_id_, event);
}

void AppEventExtensionInstance::Observe(
    const std::string& app_id, scoped_refptr<Event> event) {
  DCHECK(app_id_ == app_id);
  EventCallbackMap::iterator it = registered_events_.find(event->name());
  if (it == registered_events_.end())
    return;

  scoped_ptr<base::ListValue> args(new base::ListValue());
  args->Append(event->args()->DeepCopy());
  it->second.Run(args.Pass());
}

}  // namespace application
}  // namespace xwalk
