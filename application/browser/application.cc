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
      entry_point_used_(Default),
      weak_factory_(this) {
  DCHECK(runtime_context_);
  DCHECK(application_data_);
  DCHECK(observer_);
}

Application::~Application() {
}

bool Application::Launch(const LaunchParams& launch_params) {
  if (!runtimes_.empty()) {
    LOG(ERROR) << "Attempt to launch app: " << id()
               << " that was already launched.";
    return false;
  }

  GURL url = GetURLForLaunch(launch_params, &entry_point_used_);
  if (!url.is_valid())
    return false;

  main_runtime_ = Runtime::Create(runtime_context_, this);
  main_runtime_->LoadURL(url);
  if (entry_point_used_ != AppMainKey)
    main_runtime_->AttachDefaultWindow();

  return true;
}

GURL Application::GetURLForLaunch(const LaunchParams& params,
                                  LaunchEntryPoint* used) {
  if (params.entry_points & AppMainKey) {
    GURL url = GetURLFromAppMainKey();
    if (url.is_valid()) {
      *used = AppMainKey;
      return url;
    }
  }

  if (params.entry_points & LaunchLocalPathKey) {
    GURL url = GetURLFromLocalPathKey();
    if (url.is_valid()) {
      *used = LaunchLocalPathKey;
      return url;
    }
  }

  if (params.entry_points & URLKey) {
    GURL url = GetURLFromURLKey();
    if (url.is_valid()) {
      *used = URLKey;
      return url;
    }
  }
  LOG(WARNING) << "Failed to find a valid launch URL for the app.";
  return GURL();
}

GURL Application::GetURLFromAppMainKey() {
  MainDocumentInfo* main_info = ToMainDocumentInfo(
    application_data_->GetManifestData(keys::kAppMainKey));
  if (!main_info)
    return GURL();

  DCHECK(application_data_->HasMainDocument());
  return main_info->GetMainURL();
}

GURL Application::GetURLFromLocalPathKey() {
  const Manifest* manifest = application_data_->GetManifest();
  std::string entry_page;
  if (!manifest->GetString(keys::kLaunchLocalPathKey, &entry_page)
      || entry_page.empty())
    return GURL();

  return application_data_->GetResourceURL(entry_page);
}

GURL Application::GetURLFromURLKey() {
  const Manifest* manifest = application_data_->GetManifest();
  std::string url_string;
  if (!manifest->GetString(keys::kURLKey, &url_string))
    return GURL();

  return GURL(url_string);
}

void Application::Terminate() {
  std::set<Runtime*> to_be_closed(runtimes_);
  if (HasMainDocument() && to_be_closed.size() > 1) {
    // The main document runtime is closed separately
    // (needs some extra logic) in Application::OnRuntimeRemoved.
    to_be_closed.erase(main_runtime_);
  }

  std::set<Runtime*>::iterator it = to_be_closed.begin();
  for (; it!= to_be_closed.end(); ++it)
    (*it)->Close();
}

Runtime* Application::GetMainDocumentRuntime() const {
  return HasMainDocument() ? main_runtime_ : NULL;
}

int Application::GetRenderProcessHostID() const {
  DCHECK(main_runtime_);
  return main_runtime_->web_contents()->
          GetRenderProcessHost()->GetID();
}

bool Application::HasMainDocument() const {
  return entry_point_used_ == AppMainKey;
}

void Application::OnRuntimeAdded(Runtime* runtime) {
  DCHECK(runtime);
  runtimes_.insert(runtime);
}

void Application::OnRuntimeRemoved(Runtime* runtime) {
  DCHECK(runtime);
  runtimes_.erase(runtime);

  if (runtimes_.empty()) {
    base::MessageLoop::current()->PostTask(FROM_HERE,
        base::Bind(&Application::NotifyTermination,
                   weak_factory_.GetWeakPtr()));
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

void Application::NotifyTermination() {
  observer_->OnApplicationTerminated(this);
}

bool Application::IsOnSuspendHandlerRegistered() const {
  const std::set<std::string>& events = data()->GetEvents();
  if (events.find(kOnSuspend) == events.end())
    return false;

  return true;
}

}  // namespace application
}  // namespace xwalk
