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
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/manifest_handlers/warp_handler.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_ui_delegate.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

#if defined(OS_TIZEN)
#include "xwalk/application/browser/application_tizen.h"
#endif

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
      data->path(), true,
      base::FileEnumerator::FILES,
      FILE_PATH_LITERAL("index.*"));
  size_t priority = arraysize(kDefaultWidgetEntryPage);
  std::string source;

  for (base::FilePath file = iter.Next(); !file.empty(); file = iter.Next()) {
    for (size_t i = 0; i < priority; ++i) {
      if (file.BaseName().MaybeAsASCII() == kDefaultWidgetEntryPage[i]) {
        source = kDefaultWidgetEntryPage[i];
        priority = i;
        break;
      }
    }
    if (!priority)
      break;
  }

  return source.empty() ? GURL() : data->GetResourceURL(source);
}

}  // namespace

namespace application {

scoped_ptr<Application> Application::Create(
    scoped_refptr<ApplicationData> data,
    XWalkBrowserContext* context) {
#if defined(OS_TIZEN)
  return make_scoped_ptr<Application>(new ApplicationTizen(data, context));
#else
  return make_scoped_ptr(new Application(data, context));
#endif
}

Application::Application(
    scoped_refptr<ApplicationData> data,
    XWalkBrowserContext* browser_context)
    : data_(data),
      render_process_host_(NULL),
      web_contents_(NULL),
      security_mode_enabled_(false),
      browser_context_(browser_context),
      observer_(NULL),
      remote_debugging_enabled_(false),
      weak_factory_(this) {
  DCHECK(browser_context_);
  DCHECK(data_.get());
}

Application::~Application() {
  Terminate();
  if (render_process_host_)
    render_process_host_->RemoveObserver(this);
}

template<>
GURL Application::GetStartURL<Manifest::TYPE_WIDGET>() {
#if defined(OS_TIZEN)
  if (data_->IsHostedApp()) {
    std::string source;
    GURL url;
    if (data_->GetManifest()->GetString(
        widget_keys::kLaunchLocalPathKey, &source)) {
      url = GURL(source);
    }

    if (url.is_valid() && url.SchemeIsHTTPOrHTTPS())
      return url;
  }
#endif

  GURL url = GetAbsoluteURLFromKey(widget_keys::kLaunchLocalPathKey);
  if (url.is_valid())
    return url;

  LOG(WARNING) << "Failed to find start URL from the 'config.xml'"
               << "trying to find default entry page.";
  url = GetDefaultWidgetEntryPage(data_);
  if (url.is_valid())
    return url;

  LOG(WARNING) << "Failed to find a valid start URL in the manifest.";
  return GURL();
}

template<>
GURL Application::GetStartURL<Manifest::TYPE_MANIFEST>() {
  if (data_->IsHostedApp()) {
    std::string source;
    // Not trying to get a relative path for the "fake" application.
    if (data_->GetManifest()->GetString(keys::kStartURLKey, &source))
      return GURL(source);
    return GURL();
  }

  GURL url = GetAbsoluteURLFromKey(keys::kStartURLKey);
  if (url.is_valid())
    return url;

  url = GetAbsoluteURLFromKey(keys::kLaunchLocalPathKey);
  if (url.is_valid())
    return url;

  url = GetAbsoluteURLFromKey(keys::kDeprecatedURLKey);
  if (url.is_valid()) {
    LOG(WARNING) << "Deprecated key '" << keys::kDeprecatedURLKey
        << "' found. Please migrate to using '" << keys::kStartURLKey
        << "' instead.";
    return url;
  }

  LOG(WARNING) << "Failed to find a valid start URL in the manifest.";
  return GURL();
}


template<>
ui::WindowShowState Application::GetWindowShowState<Manifest::TYPE_WIDGET>(
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

template<>
ui::WindowShowState Application::GetWindowShowState<Manifest::TYPE_MANIFEST>(
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

bool Application::Launch(const LaunchParams& launch_params) {
  if (!runtimes_.empty()) {
    LOG(ERROR) << "Attempt to launch app with id " << id()
               << ", but it is already running.";
    return false;
  }

  CHECK(!render_process_host_);
  bool is_wgt = data_->manifest_type() == Manifest::TYPE_WIDGET;

  GURL url = is_wgt ? GetStartURL<Manifest::TYPE_WIDGET>() :
                      GetStartURL<Manifest::TYPE_MANIFEST>();
  if (!url.is_valid())
    return false;

  remote_debugging_enabled_ = launch_params.remote_debugging;
  auto site = content::SiteInstance::CreateForURL(browser_context_, url);
  Runtime* runtime = Runtime::Create(browser_context_, site);
  runtime->set_observer(this);
  runtime->set_remote_debugging_enabled(remote_debugging_enabled_);
  runtimes_.push_back(runtime);
  render_process_host_ = runtime->GetRenderProcessHost();
  render_process_host_->AddObserver(this);
  web_contents_ = runtime->web_contents();
  InitSecurityPolicy();
  runtime->LoadURL(url);

  NativeAppWindow::CreateParams params;
  params.net_wm_pid = launch_params.launcher_pid;
  params.state = is_wgt ?
      GetWindowShowState<Manifest::TYPE_WIDGET>(launch_params) :
      GetWindowShowState<Manifest::TYPE_MANIFEST>(launch_params);

  window_show_params_ = params;
  // Only the first runtime can have a launch screen.
  params.splash_screen_path = GetSplashScreenPath();
  runtime->set_ui_delegate(DefaultRuntimeUIDelegate::Create(runtime, params));
  // We call "Show" after RP is initialized to reduce
  // the application start up time.

  return true;
}

GURL Application::GetAbsoluteURLFromKey(const std::string& key) {
  const Manifest* manifest = data_->GetManifest();
  std::string source;

  if (!manifest->GetString(key, &source) || source.empty())
    return GURL();

  return data_->GetResourceURL(source);
}

void Application::Terminate() {
  std::vector<Runtime*> to_be_closed(runtimes_.get());
  for (Runtime* runtime : to_be_closed)
    runtime->Close();
}

int Application::GetRenderProcessHostID() const {
  DCHECK(render_process_host_);
  return render_process_host_->GetID();
}

void Application::OnNewRuntimeAdded(Runtime* runtime) {
  runtime->set_remote_debugging_enabled(remote_debugging_enabled_);
  runtime->set_observer(this);
  runtime->set_ui_delegate(
      DefaultRuntimeUIDelegate::Create(runtime, window_show_params_));
  runtime->Show();
  runtimes_.push_back(runtime);
}

void Application::OnRuntimeClosed(Runtime* runtime) {
  auto found = std::find(runtimes_.begin(), runtimes_.end(), runtime);
  CHECK(found != runtimes_.end());
  LOG(INFO) << "Application::OnRuntimeClosed " << runtime;
  runtimes_.erase(found);

  if (runtimes_.empty())
    base::MessageLoop::current()->PostTask(FROM_HERE,
        base::Bind(&Application::NotifyTermination,
                   weak_factory_.GetWeakPtr()));
}

void Application::RenderProcessExited(RenderProcessHost* host,
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
  if (observer_)
    observer_->OnApplicationTerminated(this);
}

void Application::RenderChannelCreated() {
  CHECK(!runtimes_.empty());
  runtimes_.front()->Show();
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
    return std::string();
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
    security_policy_.reset(new ApplicationSecurityPolicyCSP(this));
  else if (data_->manifest_type() == Manifest::TYPE_WIDGET)
    security_policy_.reset(new ApplicationSecurityPolicyWARP(this));

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
