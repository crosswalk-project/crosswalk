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
      observer_(observer) {
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
  if (!main_info || !main_info->GetMainURL().is_valid()) {
    LOG(WARNING) << "Can't find a valid main document URL for app.";
    return false;
  }

  DCHECK(HasMainDocument());
  main_runtime_ = Runtime::Create(runtime_context_, this);
  main_runtime_->LoadURL(main_info->GetMainURL());
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
bool Application::TryLaunchAt<Application::URLKey>() {
  const Manifest* manifest = application_data_->GetManifest();
  std::string url_string;
  if (manifest->GetString(application_manifest_keys::kURLKey,
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

bool Application::Launch(const LaunchParams& launch_params) {
  if (!runtimes_.empty()) {
    LOG(ERROR) << "Attempt to launch app: " << id()
               << " that was already launched.";
    return false;
  }

  if ((launch_params.entry_points & AppMainKey) && TryLaunchAt<AppMainKey>())
    return true;
  if ((launch_params.entry_points & LaunchLocalPathKey)
      && TryLaunchAt<LaunchLocalPathKey>())
    return true;
  if ((launch_params.entry_points & URLKey) && TryLaunchAt<URLKey>())
    return true;

  return false;
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
  DCHECK(main_runtime_);
  return main_runtime_->web_contents()->
          GetRenderProcessHost()->GetID();
}

bool Application::IsOnSuspendHandlerRegistered() const {
  const std::set<std::string>& events = data()->GetEvents();
  if (events.find(kOnSuspend) == events.end())
    return false;

  return true;
}

bool Application::ContainsExtension(const std::string& extension_name) const {
  // TODO(Bai): Tells whether the application contains the specified extension
  return true;
}

bool Application::RegisterPermissions(const std::string& extension_name,
                           const std::string& perm_table) {
  // TODO(Bai): Parse the permission table and fill in the name_perm_map_
  LOG(INFO) << extension_name << " registered: " << perm_table;
  return true;
}

std::string Application::GetRegisteredPermissionName(
    const std::string& extension_name,
    const std::string& api_name) const {
  std::map<std::string, std::string>::const_iterator iter;
  iter = name_perm_map_.find(api_name);
  if (iter == name_perm_map_.end())
    return std::string("");
  return iter->second;
}

StoredPermission Application::GetPermission(const PermissionType type,
                               const std::string& permission_name) const {
  StoredPermissionMap::const_iterator iter;
  if (type == SESSION_PERMISSION) {
    iter = permission_map_.find(permission_name);
    if (iter == permission_map_.end())
      return INVALID_STORED_PERM;
    return iter->second;
  }
  if (type == PERSISTENT_PERMISSION) {
    return application_data_->GetPermission(permission_name);
  }
  NOTREACHED();
  return INVALID_STORED_PERM;
}

bool Application::SetPermission(const PermissionType type,
                                const std::string& permission_name,
                                const StoredPermission perm) {
  if (type == SESSION_PERMISSION) {
    permission_map_[permission_name] = perm;
    return true;
  }
  if (type == PERSISTENT_PERMISSION)
    return application_data_->SetPermission(permission_name, perm);

  NOTREACHED();
  return false;
}

}  // namespace application
}  // namespace xwalk
