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

#if defined(OS_TIZEN)
#include "xwalk/runtime/browser/ui/native_app_window.h"
#endif

#if defined(USE_OZONE) && defined(OS_TIZEN)
#include "base/message_loop/message_pump_ozone.h"
#include "content/public/browser/render_view_host.h"
#include "ui/events/event.h"
#include "ui/events/event_constants.h"
#include "ui/events/keycodes/keyboard_codes_posix.h"
#include "xwalk/application/common/manifest_handlers/tizen_setting_handler.h"
#endif

#if defined(OS_TIZEN)
#include "xwalk/application/common/manifest_handlers/navigation_handler.h"
#endif

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
      weak_factory_(this),
      is_security_mode_(false) {
  DCHECK(runtime_context_);
  DCHECK(application_data_);
  DCHECK(observer_);
#if defined(USE_OZONE) && defined(OS_TIZEN)
  base::MessagePumpOzone::Current()->AddObserver(this);
#endif
}

Application::~Application() {
#if defined(USE_OZONE) && defined(OS_TIZEN)
  base::MessagePumpOzone::Current()->RemoveObserver(this);
#endif
  Terminate(Immediate);
}

bool Application::Launch(const LaunchParams& launch_params) {
  if (!runtimes_.empty()) {
    LOG(ERROR) << "Attempt to launch app: " << id()
               << " that was already launched.";
    return false;
  }

  GURL url = GetStartURL(launch_params, &entry_point_used_);
  if (!url.is_valid())
    return false;

  main_runtime_ = Runtime::Create(runtime_context_, this);
  InitSecurityPolicy();
  main_runtime_->LoadURL(url);
  if (entry_point_used_ != AppMainKey) {
    NativeAppWindow::CreateParams params;
    params.net_wm_pid = launch_params.launcher_pid;
    params.state = GetWindowShowState(launch_params);
    main_runtime_->AttachWindow(params);
  }

  return true;
}

GURL Application::GetStartURL(const LaunchParams& params,
                                  LaunchEntryPoint* used) {
  if (params.entry_points & URLKey) {
    GURL url = GetURLFromURLKey();
    if (url.is_valid()) {
      *used = URLKey;
      return url;
    }
  }

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

ui::WindowShowState Application::GetWindowShowState(
    const LaunchParams& params) {
  if (params.force_fullscreen)
    return ui::SHOW_STATE_FULLSCREEN;

  const Manifest* manifest = application_data_->GetManifest();
  std::string display_string;
  if (manifest->GetString(keys::kDisplay, &display_string)) {
    // FIXME: ATM only 'fullscreen' and 'standalone' (which is fallback value)
    // values are supported.
    if (display_string.find("fullscreen") != std::string::npos)
      return ui::SHOW_STATE_FULLSCREEN;
  }

  return ui::SHOW_STATE_DEFAULT;
}

GURL Application::GetURLFromLocalPathKey() {
  std::string entry_page;
  std::string key(GetLaunchLocalPathKey(
      application_data_->GetPackageType()));
  const base::DictionaryValue* manifest_data =
      application_data_->GetManifest()->value();

  base::Value* contents;
  if (application_data_->GetPackageType() == Manifest::TYPE_WGT &&
      application_data_->GetManifest()->Get(
          widget_keys::kContentKey, &contents)) {
    if (contents && contents->IsType(base::Value::TYPE_LIST)) {
      base::ListValue* list;
      base::DictionaryValue* value;
      contents->GetAsList(&list);
      for (base::ListValue::const_iterator it = list->begin();
           it != list->end(); ++it) {
        (*it)->GetAsDictionary(&value);
        std::string ns;
        if (value->GetString(widget_keys::kNamespaceKey, &ns) &&
            ns == kWidgetNamespecePrefix) {
          manifest_data = value;
          key = widget_keys::kContentSrcKey;
          break;
        }
      }
    } else if (contents && contents->IsType(base::Value::TYPE_DICTIONARY)) {
      base::DictionaryValue* value;
      contents->GetAsDictionary(&value);
      std::string ns;
      if (value->GetString(widget_keys::kNamespaceKey, &ns) &&
          ns == kWidgetNamespecePrefix) {
        manifest_data = value;
        key = widget_keys::kContentSrcKey;
      }
    }
  }

  if (!manifest_data->GetString(key, &entry_page)
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

#if defined(OS_TIZEN)
void Application::Hide() {
  DCHECK(runtimes_.size());
  std::set<Runtime*>::iterator it = runtimes_.begin();
  for (; it != runtimes_.end(); ++it) {
    if ((*it)->window())
      (*it)->window()->Hide();
  }
}
#endif

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

#if defined(OS_TIZEN)
  // On Tizen, CSP mode has higher priority, and WARP will be disabled
  // if the application is under CSP mode.
  if (application_data_->HasCSPDefined()) {
    // Always enable security mode when under CSP mode.
    is_security_mode_ = true;
    NavigationInfo* info = static_cast<NavigationInfo*>(
        application_data_->GetManifestData(widget_keys::kAllowNavigationKey));
    if (info) {
      const std::vector<std::string>& allowed_list = info->GetAllowedDomains();
      for (std::vector<std::string>::const_iterator it = allowed_list.begin();
           it != allowed_list.end(); ++it) {
        // If the policy start with "*.", like this: *.domain,
        // means that can access to all subdomains for 'domain',
        // otherwise, the host of request url should exactly the same
        // as policy.
        bool subdomains = ((*it).find("*.") == 0);
        std::string host = subdomains ? (*it).substr(2) : (*it);
        AddSecurityPolicy(GURL("http://" + host), subdomains);
        AddSecurityPolicy(GURL("https://" + host), subdomains);
      }
    }
    GetHost(main_runtime_)->Send(
        new ViewMsg_EnableSecurityMode(
            ApplicationData::GetBaseURLFromApplicationId(id()),
            SecurityPolicy::CSP));
    return;
  }
#endif
  const WARPInfo* info = static_cast<WARPInfo*>(
      application_data_->GetManifestData(widget_keys::kAccessKey));
  // FIXME(xinchao): Need to enable WARP mode by default.
  if (!info)
    return;

  const base::ListValue* whitelist = info->GetWARP();
  for (base::ListValue::const_iterator it = whitelist->begin();
       it != whitelist->end(); ++it) {
    base::DictionaryValue* value = NULL;
    (*it)->GetAsDictionary(&value);
    std::string dest;
    if (!value || !value->GetString(widget_keys::kAccessOriginKey, &dest) ||
        dest.empty())
      continue;
    if (dest == "*") {
      is_security_mode_ = false;
      break;
    }

    GURL dest_url(dest);
    // The default subdomains attrubute should be "false".
    std::string subdomains = "false";
    value->GetString(widget_keys::kAccessSubdomainsKey, &subdomains);
    AddSecurityPolicy(dest_url, (subdomains == "true"));
    is_security_mode_ = true;
  }
  if (is_security_mode_)
    GetHost(main_runtime_)->Send(
        new ViewMsg_EnableSecurityMode(
            ApplicationData::GetBaseURLFromApplicationId(id()),
            SecurityPolicy::WARP));
}

void Application::AddSecurityPolicy(const GURL& url, bool subdomains) {
  GURL app_url = application_data_->URL();
  GetHost(main_runtime_)->Send(
      new ViewMsg_SetAccessWhiteList(
          app_url, url, subdomains));
  security_policy_.push_back(new SecurityPolicy(url, subdomains));
}

bool Application::CanRequestURL(const GURL& url) const {
  if (!is_security_mode_)
    return true;

  // Only WGT package need to check the url request permission.
  if (application_data_->GetPackageType() != Manifest::TYPE_WGT)
    return true;

  // Always can request itself resources.
  if (url.SchemeIs(application::kApplicationScheme) &&
      url.host() == id())
    return true;

  for (int i = 0; i < security_policy_.size(); ++i) {
    const GURL& policy = security_policy_[i]->url();
    bool subdomains = security_policy_[i]->subdomains();
    bool is_host_matched = subdomains ?
        url.DomainIs(policy.host().c_str()) : url.host() == policy.host();
    if (url.scheme() == policy.scheme() && is_host_matched)
      return true;
  }
  return false;
}

#if defined(USE_OZONE) && defined(OS_TIZEN)
base::EventStatus Application::WillProcessEvent(
    const base::NativeEvent& event) {
  return base::EVENT_CONTINUE;
}

void Application::DidProcessEvent(
    const base::NativeEvent& event) {
  ui::Event* eve = static_cast<ui::Event*>(event);
  if (!eve->IsKeyEvent() || eve->type() != ui::ET_KEY_PRESSED)
    return;

  ui::KeyEvent* key_event = static_cast<ui::KeyEvent*>(eve);

  // FIXME: Most Wayland devices don't have similar hardware button for 'back'
  // and 'memu' as Tizen Mobile, even that hardare buttons could be different
  // across different kinds of Wayland platforms.
  // Here use external keyboard button 'Backspace' & 'HOME' to emulate 'back'
  // and 'menu' key. Should change this if there is customized key binding.
  if (key_event->key_code() != ui::VKEY_BACK &&
      key_event->key_code() != ui::VKEY_HOME)
    return;

  TizenSettingInfo* info = static_cast<TizenSettingInfo*>(
      data()->GetManifestData(widget_keys::kTizenSettingKey));
  if (info && !info->hwkey_enabled())
    return;

  for (std::set<xwalk::Runtime*>::iterator it = runtimes_.begin();
      it != runtimes_.end(); ++it) {
    (*it)->web_contents()->GetRenderViewHost()->Send(new ViewMsg_HWKeyPressed(
        (*it)->web_contents()->GetRoutingID(), key_event->key_code()));
  }
}
#endif

}  // namespace application
}  // namespace xwalk
