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
#include "base/strings/string_split.h"
#include "base/threading/thread_restrictions.h"
#include "base/values.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/site_instance.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/application_storage.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/manifest_handlers/warp_handler.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

using content::RenderProcessHost;

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

GURL GetDefaultWidgetEntryPage(
    scoped_refptr<xwalk::application::ApplicationData> data) {
  base::ThreadRestrictions::SetIOAllowed(true);
  base::FileEnumerator iter(
      data->Path(), true,
      base::FileEnumerator::FILES,
      FILE_PATH_LITERAL("index.*"));
  size_t priority = arraysize(kDefaultWidgetEntryPage);
  std::string source;

  for (base::FilePath file = iter.Next(); !file.empty(); file = iter.Next()) {
    for (size_t i = 0; i < arraysize(kDefaultWidgetEntryPage); ++i) {
      if (file.BaseName().MaybeAsASCII() == kDefaultWidgetEntryPage[i] &&
          i < priority) {
        source = kDefaultWidgetEntryPage[i];
        priority = i;
      }
    }
  }

  return source.empty() ? GURL() : data->GetResourceURL(source);
}

}  // namespace

namespace application {

Application::Application(
    scoped_refptr<ApplicationData> data,
    RuntimeContext* runtime_context,
    Observer* observer)
    : data_(data),
      render_process_host_(NULL),
      web_contents_(NULL),
      security_mode_enabled_(false),
      runtime_context_(runtime_context),
      observer_(observer),
      entry_point_used_(Default),
      remote_debugging_enabled_(false),
      weak_factory_(this) {
  DCHECK(runtime_context_);
  DCHECK(data_.get());
  DCHECK(observer_);
}

Application::~Application() {
  Terminate();
  if (render_process_host_)
    render_process_host_->RemoveObserver(this);
}

bool Application::Launch(const LaunchParams& launch_params) {
  if (!runtimes_.empty()) {
    LOG(ERROR) << "Attempt to launch app: " << id()
               << " that was already launched.";
    return false;
  }

  CHECK(!render_process_host_);

  GURL url = GetStartURL(launch_params, &entry_point_used_);
  if (!url.is_valid())
    return false;

  remote_debugging_enabled_ = launch_params.remote_debugging;

  Runtime* runtime = Runtime::Create(
      runtime_context_,
      this, content::SiteInstance::CreateForURL(runtime_context_, url));
  render_process_host_ = runtime->GetRenderProcessHost();
  render_process_host_->AddObserver(this);
  web_contents_ = runtime->web_contents();
  InitSecurityPolicy();
  runtime->LoadURL(url);

  NativeAppWindow::CreateParams params;
  params.net_wm_pid = launch_params.launcher_pid;
  if (data_->GetPackageType() == Package::WGT)
    params.state = GetWindowShowStateWGT(launch_params);
  else
    params.state = GetWindowShowStateXPK(launch_params);

  params.splash_screen_path = GetSplashScreenPath();

  runtime->AttachWindow(params);

  return true;
}

GURL Application::GetStartURL(const LaunchParams& params,
    LaunchEntryPoint* used) {
  if (params.entry_points & StartURLKey) {
    GURL url = GetAbsoluteURLFromKey(keys::kStartURLKey);
    if (url.is_valid()) {
      *used = StartURLKey;
      return url;
    }
  }

  if (params.entry_points & LaunchLocalPathKey) {
    GURL url = GetAbsoluteURLFromKey(
        GetLaunchLocalPathKey(data_->GetPackageType()));

    if (!url.is_valid() && data_->GetPackageType() == Package::WGT)
      url = GetDefaultWidgetEntryPage(data_);

    if (url.is_valid()) {
#if defined(OS_TIZEN)
      if (data_->IsHostedApp() && !url.SchemeIsHTTPOrHTTPS()) {
        LOG(ERROR) << "Hosted application should use the url start with"
                      "http or https as its entry page.";
        return GURL();
      }
#endif
      *used = LaunchLocalPathKey;
      return url;
    }
  }

  if (params.entry_points & URLKey) {
    LOG(WARNING) << "Deprecated key '" << keys::kDeprecatedURLKey
        << "' found. Please migrate to using '" << keys::kStartURLKey
        << "' instead.";
    GURL url = GetAbsoluteURLFromKey(keys::kDeprecatedURLKey);
    if (url.is_valid()) {
      *used = URLKey;
      return url;
    }
  }

  LOG(WARNING) << "Failed to find a valid start URL in the manifest.";
  return GURL();
}

ui::WindowShowState Application::GetWindowShowStateWGT(
    const LaunchParams& params) {
  if (params.force_fullscreen)
    return ui::SHOW_STATE_FULLSCREEN;

  const Manifest* manifest = data_->GetManifest();
  std::string view_modes_string;
  if (manifest->GetString(widget_keys::kViewModesKey, &view_modes_string)) {
    // FIXME: ATM only 'fullscreen' and 'windowed' values are supported.
    // If the first user prefererence is 'fullscreen', set window show state
    // FULLSCREEN, otherwise set the default window show state.
    std::vector<std::string> modes;
    base::SplitString(view_modes_string, ' ', &modes);
    if (!modes.empty() && modes[0] == "fullscreen")
      return ui::SHOW_STATE_FULLSCREEN;
  }

  return ui::SHOW_STATE_DEFAULT;
}

ui::WindowShowState Application::GetWindowShowStateXPK(
    const LaunchParams& params) {
  if (params.force_fullscreen)
    return ui::SHOW_STATE_FULLSCREEN;

  const Manifest* manifest = data_->GetManifest();
  std::string display_string;
  if (manifest->GetString(keys::kDisplay, &display_string)) {
    // FIXME: ATM only 'fullscreen' and 'standalone' (which is fallback value)
    // values are supported.
    if (display_string.find("fullscreen") != std::string::npos)
      return ui::SHOW_STATE_FULLSCREEN;
  }

  return ui::SHOW_STATE_DEFAULT;
}

GURL Application::GetAbsoluteURLFromKey(const std::string& key) {
  const Manifest* manifest = data_->GetManifest();
  std::string source;

  if (!manifest->GetString(key, &source) || source.empty())
    return GURL();

  std::size_t found = source.find_first_of("://");
  if (found == std::string::npos)
    return data_->GetResourceURL(source);
  return GURL(source);
}

void Application::Terminate() {
  std::set<Runtime*> to_be_closed(runtimes_);
  std::for_each(to_be_closed.begin(), to_be_closed.end(),
                std::mem_fun(&Runtime::Close));
}

int Application::GetRenderProcessHostID() const {
  DCHECK(render_process_host_);
  return render_process_host_->GetID();
}

void Application::OnRuntimeAdded(Runtime* runtime) {
  DCHECK(runtime);
  runtime->set_remote_debugging_enabled(remote_debugging_enabled_);
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
}

void Application::RenderProcessExited(RenderProcessHost* host,
                                      base::ProcessHandle,
                                      base::TerminationStatus,
                                      int) {
  DCHECK(render_process_host_ == host);
  VLOG(1) << "RenderProcess id: " << host->GetID() << " is gone!";
  XWalkRunner::GetInstance()->OnRenderProcessHostGone(host);
}

void Application::RenderProcessHostDestroyed(RenderProcessHost* host) {
  DCHECK(render_process_host_ == host);
  render_process_host_ = NULL;
  web_contents_ = NULL;
}

void Application::NotifyTermination() {
  CHECK(!render_process_host_);
  observer_->OnApplicationTerminated(this);
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
                               const std::string& permission_name) const {
  if (type == SESSION_PERMISSION) {
    StoredPermissionMap::const_iterator iter =
        permission_map_.find(permission_name);
    if (iter == permission_map_.end())
      return UNDEFINED_STORED_PERM;
    return iter->second;
  }
  if (type == PERSISTENT_PERMISSION) {
    return data_->GetPermission(permission_name);
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
    return data_->SetPermission(permission_name, perm);

  NOTREACHED();
  return false;
}

void Application::InitSecurityPolicy() {
  // CSP policy takes precedence over WARP.
  if (data_->HasCSPDefined())
    security_policy_.reset(new SecurityPolicyCSP(this));
  else if (data_->GetPackageType() == Package::WGT)
    security_policy_.reset(new SecurityPolicyWARP(this));

  if (security_policy_)
    security_policy_->Enforce();
}

bool Application::CanRequestURL(const GURL& url) const {
  if (security_policy_)
    return security_policy_->IsAccessAllowed(url);
  return true;
}

base::FilePath Application::GetSplashScreenPath() {
  return base::FilePath();
}

}  // namespace application
}  // namespace xwalk
