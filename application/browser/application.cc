// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application.h"

#include <string>

#include "base/files/file_enumerator.h"
#include "base/json/json_reader.h"
#include "base/macros.h"
#include "base/message_loop/message_loop.h"
#include "base/stl_util.h"
#include "base/values.h"
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
#include "xwalk/application/common/manifest_handlers/warp_handler.h"
#include "xwalk/application/common/event_names.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_common_messages.h"

namespace xwalk {

namespace keys = application_manifest_keys;
namespace widget_keys = application_widget_keys;

namespace {
const char* kDefaultWidgetEntryPage[] = {
"index.html",
"index.htm",
"index.svg",
"index.xhtml",
"index.xht"};

content::RenderProcessHost* GetHost(Runtime* runtime) {
  DCHECK(runtime);
  return runtime->web_contents()->GetRenderProcessHost();
}
}  // namespace

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
      termination_mode_used_(Normal),
      weak_factory_(this) {
  DCHECK(runtime_context_);
  DCHECK(application_data_);
  DCHECK(observer_);
}

Application::~Application() {
  Terminate(Immediate);
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
  InitSecurityPolicy();
  main_runtime_->LoadURL(url);
  if (entry_point_used_ != AppMainKey) {
    NativeAppWindow::CreateParams params;
    params.net_wm_pid = launch_params.launcher_pid;
    params.state = launch_params.window_state;

    main_runtime_->AttachWindow(params);
  }

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
  std::string key(GetLaunchLocalPathKey(
      application_data_->GetPackageType()));

  if (!manifest->GetString(key, &entry_page)
      || entry_page.empty()) {
    if (application_data_->GetPackageType() == Manifest::TYPE_XPK)
      return GURL();

    base::FileEnumerator iter(application_data_->Path(), true,
                              base::FileEnumerator::FILES,
                              FILE_PATH_LITERAL("index.*"));
    int priority = arraysize(kDefaultWidgetEntryPage);

    for (base::FilePath file = iter.Next(); !file.empty(); file = iter.Next()) {
      for (int i = 0; i < arraysize(kDefaultWidgetEntryPage); ++i) {
        if (file.BaseName().MaybeAsASCII() == kDefaultWidgetEntryPage[i] &&
            i < priority) {
          entry_page = kDefaultWidgetEntryPage[i];
          priority = i;
        }
      }
    }

    if (entry_page.empty())
      return GURL();
  }

  return application_data_->GetResourceURL(entry_page);
}

GURL Application::GetURLFromURLKey() {
  const Manifest* manifest = application_data_->GetManifest();
  std::string url_string;
  if (!manifest->GetString(keys::kURLKey, &url_string))
    return GURL();

  return GURL(url_string);
}

void Application::Terminate(TerminationMode mode) {
  termination_mode_used_ = mode;
  if (IsTerminating()) {
    LOG(WARNING) << "Attempt to Terminate app: " << id()
                 << ", which is already in the process of being terminated.";
    if (mode == Immediate)
      CloseMainDocument();

    return;
  }

  std::set<Runtime*> to_be_closed(runtimes_);
  if (HasMainDocument() && to_be_closed.size() > 1) {
    // The main document runtime is closed separately
    // (needs some extra logic) in Application::OnRuntimeRemoved.
    to_be_closed.erase(main_runtime_);
  }

  std::for_each(to_be_closed.begin(), to_be_closed.end(),
                std::mem_fun(&Runtime::Close));
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
#if defined(OS_TIZEN_MOBILE)
    runtime->CloseRootWindow();
#endif
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
    if (!IsOnSuspendHandlerRegistered() ||
        termination_mode_used_ == Immediate) {
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

bool Application::UseExtension(const std::string& extension_name) const {
  // TODO(Bai): Tells whether the application contains the specified extension
  return true;
}

bool Application::RegisterPermissions(const std::string& extension_name,
                                      const std::string& perm_table) {
  // TODO(Bai): Parse the permission table and fill in the name_perm_map_
  // The perm_table format is a simple JSON string, like
  // [{"permission_name":"echo","apis":["add","remove","get"]}]
  scoped_ptr<base::Value> root;
  root.reset(base::JSONReader().ReadToValue(perm_table));
  if (root.get() == NULL || !root->IsType(base::Value::TYPE_LIST))
    return false;

  base::ListValue* permission_list = static_cast<base::ListValue*>(root.get());
  if (permission_list->GetSize() == 0)
    return false;

  for (base::ListValue::const_iterator iter = permission_list->begin();
      iter != permission_list->end(); ++iter) {
    if (!(*iter)->IsType(base::Value::TYPE_DICTIONARY))
        return false;

    base::DictionaryValue* dict_val =
        static_cast<base::DictionaryValue*>(*iter);
    std::string permission_name;
    if (!dict_val->GetString("permission_name", &permission_name))
      return false;

    base::ListValue* api_list = NULL;
    if (!dict_val->GetList("apis", &api_list))
      return false;

    for (base::ListValue::const_iterator api_iter = api_list->begin();
        api_iter != api_list->end(); ++api_iter) {
      std::string api;
      if (!((*api_iter)->IsType(base::Value::TYPE_STRING)
          && (*api_iter)->GetAsString(&api)))
        return false;
      // register the permission and api
      name_perm_map_[api] = permission_name;
      DLOG(INFO) << "Permission Registered [PERM] " << permission_name
                 << " [API] " << api;
    }
  }

  return true;
}

std::string Application::GetRegisteredPermissionName(
    const std::string& extension_name,
    const std::string& api_name) const {
  std::map<std::string, std::string>::const_iterator iter =
      name_perm_map_.find(api_name);
  if (iter == name_perm_map_.end())
    return std::string("");
  return iter->second;
}

StoredPermission Application::GetPermission(PermissionType type,
                               std::string& permission_name) const {
  if (type == SESSION_PERMISSION) {
    StoredPermissionMap::const_iterator iter =
        permission_map_.find(permission_name);
    if (iter == permission_map_.end())
      return UNDEFINED_STORED_PERM;
    return iter->second;
  }
  if (type == PERSISTENT_PERMISSION) {
    return application_data_->GetPermission(permission_name);
  }
  NOTREACHED();
  return UNDEFINED_STORED_PERM;
}

bool Application::SetPermission(PermissionType type,
                                const std::string& permission_name,
                                StoredPermission perm) {
  if (type == SESSION_PERMISSION) {
    permission_map_[permission_name] = perm;
    return true;
  }
  if (type == PERSISTENT_PERMISSION)
    return application_data_->SetPermission(permission_name, perm);

  NOTREACHED();
  return false;
}

void Application::InitSecurityPolicy() {
  if (application_data_->GetPackageType() != Manifest::TYPE_WGT)
    return;
  const WARPInfo* info = static_cast<WARPInfo*>(
      application_data_->GetManifestData(widget_keys::kAccessKey));
  if (!info
#if defined(OS_TIZEN)
      // On Tizen, CSP mode has higher priority, and WARP will be disabled
      // if the application is under CSP mode.
      || application_data_->HasCSPDefined()
#endif
      )
    return;
  GURL app_url = application_data_->URL();
  const base::ListValue* whitelist = info->GetWARP();
  bool enable_warp_mode = true;
  for (base::ListValue::const_iterator it = whitelist->begin();
       it != whitelist->end(); ++it) {
    base::DictionaryValue* value = NULL;
    (*it)->GetAsDictionary(&value);
    std::string dest;
    if (!value || !value->GetString(widget_keys::kAccessOriginKey, &dest) ||
        dest.empty())
      continue;
    if (dest == "*") {
      enable_warp_mode = false;
      break;
    }

    GURL dest_url(dest);
    // The default subdomains attrubute should be "false".
    std::string subdomains = "false";
    value->GetString(widget_keys::kAccessSubdomainsKey, &subdomains);
    GetHost(main_runtime_)->Send(
        new ViewMsg_SetAccessWhiteList(
            app_url, dest_url, (subdomains == "true")));
  }
  if (enable_warp_mode)
    GetHost(main_runtime_)->Send(new ViewMsg_EnableWarpMode());
}

}  // namespace application
}  // namespace xwalk
