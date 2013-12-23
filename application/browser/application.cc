// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application.h"

#include <string>

#include "base/message_loop/message_loop.h"
#include "base/stl_util.h"
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
    scoped_refptr<const ApplicationData> data,
    RuntimeContext* runtime_context)
    : runtime_context_(runtime_context),
      application_data_(data),
      main_runtime_(NULL),
      weak_ptr_factory_(this) {
  DCHECK(application_data_);
}

Application::~Application() {
}

bool Application::Launch() {
  if (is_launched()) {
    LOG(ERROR) << "Attempt to launch app: " << id()
               << " that was already launched.";
    return false;
  }

  if (RunMainDocument())
    return true;
  // NOTE: For now we allow launching a web app from a local path. This may go
  // away at some point.
  return RunFromLocalPath();
}

void Application::Close() {
  std::set<Runtime*> cached(runtimes_);
  std::set<Runtime*>::iterator it = cached.begin();
  for (; it!= cached.end(); ++it)
    if (main_runtime_ != *it)
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
    // FIXME(Mikhail): this should go away. We will notify ApplicationService
    // about termination instead.
    base::MessageLoop::current()->PostTask(
          FROM_HERE, base::MessageLoop::QuitClosure());
    return;
  }

  if (runtimes_.size() == 1 &&
      ContainsKey(runtimes_, main_runtime_)) {
    ApplicationSystem* system = runtime_context_->GetApplicationSystem();
    ApplicationEventManager* event_manager = system->event_manager();

    // If onSuspend is not registered in main document,
    // we close the main document immediately.
    if (!IsOnSuspendHandlerRegistered(application_data_->ID())) {
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

bool Application::RunMainDocument() {
  const MainDocumentInfo* main_info =
      ToMainDocumentInfo(application_data_->GetManifestData(keys::kAppMainKey));
  if (!main_info || !main_info->GetMainURL().is_valid())
    return false;

  main_runtime_ = Runtime::Create(runtime_context_,
                                  main_info->GetMainURL(), this);
  ApplicationEventManager* event_manager =
      runtime_context_->GetApplicationSystem()->event_manager();
  event_manager->OnMainDocumentCreated(
      application_data_->ID(), main_runtime_->web_contents());
  return true;
}

void Application::CloseMainDocument() {
  DCHECK(main_runtime_);

  finish_observer_.reset();
  main_runtime_->Close();
  main_runtime_ = NULL;
}

bool Application::RunFromLocalPath() {
  const Manifest* manifest = application_data_->GetManifest();
  std::string entry_page;
  if (manifest->GetString(application_manifest_keys::kLaunchLocalPathKey,
        &entry_page) && !entry_page.empty()) {
    GURL url = application_data_->GetResourceURL(entry_page);
    if (url.is_empty()) {
      LOG(WARNING) << "Can't find a valid local path URL for app.";
      return false;
    }

    Runtime::CreateWithDefaultWindow(runtime_context_, url, this);
    return true;
  }

  return false;
}

bool Application::IsOnSuspendHandlerRegistered(
    const std::string& app_id) const {
  ApplicationSystem* system = runtime_context_->GetApplicationSystem();
  ApplicationStorage* storage = system->application_storage();

  const std::set<std::string>& events =
      storage->GetApplicationData(app_id)->GetEvents();
  if (events.find(kOnSuspend) == events.end())
    return false;

  return true;
}

}  // namespace application
}  // namespace xwalk
