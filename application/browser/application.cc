// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application.h"

#include <string>

#include "base/message_loop/message_loop.h"
#include "base/stl_util.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_process_host.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_storage.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/manifest_handlers/main_document_handler.h"
#include "xwalk/application/common/event_names.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

namespace xwalk {

namespace keys = application_manifest_keys;

namespace application {

class FinishEventObserver : public EventObserver {
 public:
  FinishEventObserver(
      ApplicationEventManager* event_manager,
      Application* application)
      : EventObserver(event_manager),
        application_(application) {
  }

  virtual void Observe(const std::string& app_id,
                       scoped_refptr<Event> event) OVERRIDE {
    DCHECK(xwalk::application::kOnJavaScriptEventAck == event->name());
    std::string ack_event_name;
    event->args()->GetString(0, &ack_event_name);
    if (ack_event_name == xwalk::application::kOnSuspend)
      application_->CloseMainDocument();
  }

 private:
  Application* application_;
};

Application::Application(
    scoped_refptr<ApplicationData> data,
    RuntimeContext* runtime_context,
    Observer* observer)
    : runtime_context_(runtime_context),
      application_data_(data),
      main_runtime_(NULL),
      observer_(observer),
      entry_points_(Default) {
  DCHECK(runtime_context_);
  DCHECK(application_data_);
  DCHECK(observer_);
}

Application::~Application() {
}

template<>
bool Application::TryLaunchAt<Application::AppMainKey>() {
  const MainDocumentInfo* main_info =
      ToMainDocumentInfo(application_data_->GetManifestData(keys::kAppMainKey));
  if (!main_info || !main_info->GetMainURL().is_valid())
    return false;

  main_runtime_ = Runtime::Create(runtime_context_, this);
  main_runtime_->LoadURL(main_info->GetMainURL());

  ApplicationEventManager* event_manager =
      XWalkRunner::GetInstance()->app_system()->event_manager();
  event_manager->OnMainDocumentCreated(
      application_data_->ID(), main_runtime_->web_contents());
  return true;
}

template<>
bool Application::TryLaunchAt<Application::LaunchLocalPathKey>() {
  const Manifest* manifest = application_data_->GetManifest();
  std::string entry_page;
  if (manifest->GetString(application_manifest_keys::kLaunchLocalPathKey,
        &entry_page) && !entry_page.empty()) {
    GURL url = application_data_->GetResourceURL(entry_page);
    if (url.is_empty()) {
      LOG(WARNING) << "Can't find a valid local path URL for app.";
      return false;
    }

    // main_runtime_ should be initialized before 'LoadURL' call,
    // so that it is in place already when application extensions
    // are created.
    main_runtime_= Runtime::Create(runtime_context_, this);
    main_runtime_->LoadURL(url);
    main_runtime_->AttachDefaultWindow();
    return true;
  }

  return false;
}

template<>
bool Application::TryLaunchAt<Application::LaunchWebURLKey>() {
  const Manifest* manifest = application_data_->GetManifest();
  std::string url_string;
  if (manifest->GetString(application_manifest_keys::kLaunchWebURLKey,
      &url_string)) {
    GURL url(url_string);
    if (!url.is_valid()) {
      LOG(WARNING) << "Can't find a valid URL for app.";
      return false;
    }

    // main_runtime_ should be initialized before 'LoadURL' call,
    // so that it is in place already when application extensions
    // are created.
    main_runtime_= Runtime::Create(runtime_context_, this);
    main_runtime_->LoadURL(url);
    main_runtime_->AttachDefaultWindow();
    return true;
  }

  return false;
}

bool Application::Launch() {
  if (!runtimes_.empty()) {
    LOG(ERROR) << "Attempt to launch app: " << id()
               << " that was already launched.";
    return false;
  }

  if ((entry_points_ & AppMainKey) && TryLaunchAt<AppMainKey>())
    return true;
  if ((entry_points_ & LaunchLocalPathKey) && TryLaunchAt<LaunchLocalPathKey>())
    return true;
  if ((entry_points_ & LaunchWebURLKey) && TryLaunchAt<LaunchWebURLKey>())
    return true;

  return false;
}

void Application::set_entry_points(LaunchEntryPoints entry_points) {
  entry_points_ = entry_points;
}

void Application::Close() {
  std::set<Runtime*> cached(runtimes_);
  std::set<Runtime*>::iterator it = cached.begin();
  for (; it!= cached.end(); ++it)
    if (!HasMainDocument() || main_runtime_ != *it)
      (*it)->Close();
}

void Application::OnRuntimeAdded(Runtime* runtime) {
  DCHECK(runtime);
  runtimes_.insert(runtime);
}

void Application::OnRuntimeRemoved(Runtime* runtime) {
  DCHECK(runtime);
  runtimes_.erase(runtime);

  if (runtimes_.empty()) {
    observer_->OnApplicationTerminated(this);
    return;
  }

  if (runtimes_.size() == 1 && HasMainDocument() &&
      ContainsKey(runtimes_, main_runtime_)) {
    ApplicationSystem* system = XWalkRunner::GetInstance()->app_system();
    ApplicationEventManager* event_manager = system->event_manager();

    // If onSuspend is not registered in main document,
    // we close the main document immediately.
    if (!IsOnSuspendHandlerRegistered()) {
      CloseMainDocument();
      return;
    }

    DCHECK(!finish_observer_);
    finish_observer_.reset(
        new FinishEventObserver(event_manager, this));
    event_manager->AttachObserver(
        application_data_->ID(), kOnJavaScriptEventAck,
        finish_observer_.get());

    scoped_ptr<base::ListValue> event_args(new base::ListValue);
    scoped_refptr<Event> event = Event::CreateEvent(
        xwalk::application::kOnSuspend, event_args.Pass());
    event_manager->SendEvent(application_data_->ID(), event);
  }
}

void Application::CloseMainDocument() {
  DCHECK(main_runtime_);

  finish_observer_.reset();
  main_runtime_->Close();
  main_runtime_ = NULL;
}

Runtime* Application::GetMainDocumentRuntime() const {
  return HasMainDocument() ? main_runtime_ : NULL;
}

int Application::GetRenderProcessHostID() const {
  return main_runtime_->web_contents()->
          GetRenderProcessHost()->GetID();
}

bool Application::IsOnSuspendHandlerRegistered() const {
  ApplicationSystem* system = XWalkRunner::GetInstance()->app_system();
  ApplicationStorage* storage = system->application_storage();

  const std::set<std::string>& events = data()->GetEvents();
  if (events.find(kOnSuspend) == events.end())
    return false;

  return true;
}

}  // namespace application
}  // namespace xwalk
