// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_process_manager.h"

#include <string>

#include "base/stl_util.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/manifest_handlers/main_document_handler.h"
#include "xwalk/application/common/event_names.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"

using xwalk::Runtime;
using xwalk::RuntimeContext;

namespace xwalk {

namespace keys = application_manifest_keys;

namespace application {

class FinishEventObserver : public EventObserver {
 public:
  FinishEventObserver(
      ApplicationEventManager* event_manager,
      ApplicationProcessManager* process_manager)
      : EventObserver(event_manager),
        process_manager_(process_manager) {
  }

  virtual void Observe(const std::string& app_id,
                       scoped_refptr<Event> event) OVERRIDE {
    DCHECK(xwalk::application::kOnJavaScriptEventAck == event->name());
    std::string ack_event_name;
    event->args()->GetString(0, &ack_event_name);
    if (ack_event_name == xwalk::application::kOnSuspend)
      process_manager_->CloseMainDocument();
  }


 private:
  ApplicationProcessManager* process_manager_;
};

ApplicationProcessManager::ApplicationProcessManager(
    RuntimeContext* runtime_context)
    : runtime_context_(runtime_context),
      main_runtime_(NULL),
      weak_ptr_factory_(this) {
}

ApplicationProcessManager::~ApplicationProcessManager() {
}

bool ApplicationProcessManager::LaunchApplication(
        RuntimeContext* runtime_context,
        const ApplicationData* application) {
  if (RunMainDocument(application))
    return true;
  // NOTE: For now we allow launching a web app from a local path. This may go
  // away at some point.
  return RunFromLocalPath(application);
}

void ApplicationProcessManager::OnRuntimeAdded(Runtime* runtime) {
  DCHECK(runtime);
  runtimes_.insert(runtime);
}

void ApplicationProcessManager::OnRuntimeRemoved(Runtime* runtime) {
  DCHECK(runtime);
  runtimes_.erase(runtime);
  // FIXME: main_runtime_ should always be the last one to close
  // in RuntimeRegistry. Need to fix the issue from browser tests.
  if (runtimes_.size() == 1 &&
      ContainsKey(runtimes_, main_runtime_)) {
    ApplicationSystem* system = runtime_context_->GetApplicationSystem();
    ApplicationEventManager* event_manager = system->event_manager();
    ApplicationService* service = system->application_service();

    scoped_refptr<const ApplicationData> app_data =
        service->GetRunningApplication();
    const MainDocumentInfo* main_info =
        ToMainDocumentInfo(app_data->GetManifestData(keys::kAppMainKey));
    DCHECK(main_info);
    if (main_info->IsPersistent())
      return;

    std::string app_id = app_data->ID();

    // If onSuspend is not registered in main document,
    // we close the main document immediately.
    if (!IsOnSuspendHandlerRegistered(app_id)) {
      CloseMainDocument();
      return;
    }

    DCHECK(!finish_observer_);
    finish_observer_.reset(
        new FinishEventObserver(event_manager, this));
    event_manager->AttachObserver(
      app_id, kOnJavaScriptEventAck,
      finish_observer_.get());

    scoped_ptr<base::ListValue> event_args(new base::ListValue);
    scoped_refptr<Event> event = Event::CreateEvent(
        xwalk::application::kOnSuspend, event_args.Pass());
    event_manager->SendEvent(app_id, event);
  }
}

bool ApplicationProcessManager::RunMainDocument(
    const ApplicationData* application) {
  const MainDocumentInfo* main_info =
      ToMainDocumentInfo(application->GetManifestData(keys::kAppMainKey));
  if (!main_info || !main_info->GetMainURL().is_valid())
    return false;

  main_runtime_ = Runtime::Create(runtime_context_, main_info->GetMainURL());
  ApplicationEventManager* event_manager =
      runtime_context_->GetApplicationSystem()->event_manager();
  event_manager->OnMainDocumentCreated(
      application->ID(), main_runtime_->web_contents());
  return true;
}

void ApplicationProcessManager::CloseMainDocument() {
  DCHECK(main_runtime_);

  finish_observer_.reset();
  main_runtime_->Close();
  main_runtime_ = NULL;
}

bool ApplicationProcessManager::RunFromLocalPath(
    const ApplicationData* application) {
  const Manifest* manifest = application->GetManifest();
  std::string entry_page;
  if (manifest->GetString(keys::kLaunchLocalPathKey, &entry_page) &&
      !entry_page.empty()) {
    GURL url = application->GetResourceURL(entry_page);
    if (url.is_empty()) {
      LOG(WARNING) << "Can't find a valid local path URL for app.";
      return false;
    }

    Runtime::CreateWithDefaultWindow(runtime_context_, url);
    return true;
  }

  return false;
}

bool ApplicationProcessManager::IsOnSuspendHandlerRegistered(
    const std::string& app_id) const {
  ApplicationSystem* system = runtime_context_->GetApplicationSystem();
  ApplicationService* service = system->application_service();

  const std::set<std::string>& events =
      service->GetApplicationByID(app_id)->GetEvents();
  if (events.find(kOnSuspend) == events.end())
    return false;

  return true;
}

}  // namespace application
}  // namespace xwalk
