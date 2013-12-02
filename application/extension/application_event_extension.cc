// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/extension/application_event_extension.h"

#include "grit/xwalk_application_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/application.h"
#include "xwalk/application/common/event_names.h"

namespace xwalk {

using application::Event;

ApplicationEventExtension::ApplicationEventExtension(
    application::ApplicationSystem* system)
  : application_system_(system) {
  set_name("xwalk.app.events");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_APPLICATION_EVENT_API).as_string());
}

XWalkExtensionInstance* ApplicationEventExtension::CreateInstance() {
  const application::ApplicationService* service =
    application_system_->application_service();
  // FIXME: return corresponding application info after shared runtime process
  // model is enabled.
  const application::Application* app = service->GetRunningApplication();
  CHECK(app);
  return new AppEventExtensionInstance(
      application_system_->event_manager(), app->ID());
}

AppEventExtensionInstance::AppEventExtensionInstance(
    application::ApplicationEventManager* event_manager,
    const std::string& app_id)
  : application::EventObserver(event_manager),
    app_id_(app_id),
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

void AppEventExtensionInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  handler_.HandleMessage(msg.Pass());
}

void AppEventExtensionInstance::OnRegisterEvent(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  std::string event_name;
  if (info->arguments()->GetSize() != 1 ||
      !info->arguments()->GetString(0, &event_name))
    return;

  if (!ContainsKey(registered_events_, event_name)) {
    registered_events_.insert(
        std::make_pair(event_name, info->post_result_cb()));
    event_manager_->AttachObserver(app_id_, event_name, this);
  }
}

void AppEventExtensionInstance::OnUnregisterEvent(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  std::string event_name;
  if (info->arguments()->GetSize() != 1 ||
      !info->arguments()->GetString(0, &event_name))
    return;

  if (!ContainsKey(registered_events_, event_name))
    return;

  registered_events_.erase(event_name);
  event_manager_->DetachObserver(app_id_, event_name, this);
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
  event_manager_->SendEvent(app_id_, event);
}

void AppEventExtensionInstance::Observe(
    const std::string& app_id, scoped_refptr<application::Event> event) {
  DCHECK(app_id_ == app_id);
  EventCallbackMap::iterator it = registered_events_.find(event->name());
  if (it == registered_events_.end())
    return;

  scoped_ptr<base::ListValue> args(new base::ListValue());
  args->Append(event->args()->DeepCopy());
  it->second.Run(args.Pass());
}

}  // namespace xwalk
