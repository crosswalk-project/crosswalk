// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application.h"

#include <string>

#include "base/files/file_enumerator.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop.h"
#include "base/stl_util.h"
#include "base/strings/string_split.h"
#include "base/threading/thread_restrictions.h"
#include "base/values.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/site_instance.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/manifest_handlers/warp_handler.h"
#include "xwalk/application/common/package/wgt_package.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_platform_util.h"
#include "xwalk/runtime/browser/runtime_ui_delegate.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

using content::RenderProcessHost;

namespace xwalk {

namespace keys = application_manifest_keys;
namespace values = application_manifest_values;
namespace widget_keys = application_widget_keys;

namespace {

GURL GetDefaultWidgetEntryPage(
    scoped_refptr<xwalk::application::ApplicationData> data) {
  base::ThreadRestrictions::SetIOAllowed(true);
  base::FileEnumerator iter(
      data->path(), true,
      base::FileEnumerator::FILES,
      FILE_PATH_LITERAL("index.*"));
  const std::vector<std::string>& defaultWidgetEntryPages =
      application::WGTPackage::GetDefaultWidgetEntryPages();
  size_t priority = defaultWidgetEntryPages.size();
  std::string source;

  for (base::FilePath file = iter.Next(); !file.empty(); file = iter.Next()) {
    for (size_t i = 0; i < priority; ++i) {
      if (file.BaseName().MaybeAsASCII() == defaultWidgetEntryPages[i]) {
        source = defaultWidgetEntryPages[i];
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

std::unique_ptr<Application> Application::Create(
    scoped_refptr<ApplicationData> data,
    XWalkBrowserContext* context) {
  return base::WrapUnique(new Application(data, context));
}

Application::Application(
    scoped_refptr<ApplicationData> data,
    XWalkBrowserContext* browser_context)
    : browser_context_(browser_context),
      data_(data),
      render_process_host_(NULL),
      web_contents_(NULL),
      security_mode_enabled_(false),
      observer_(NULL),
      security_policy_(ApplicationSecurityPolicy::Create(data_)),
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
GURL Application::GetStartURL<Manifest::TYPE_WIDGET>() const {
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
GURL Application::GetStartURL<Manifest::TYPE_MANIFEST>() const {
  if (data_->IsHostedApp()) {
    std::string source;
    // Not trying to get a relative path for the "fake" application.
    if (data_->GetManifest()->GetString(keys::kStartURLKey, &source))
      return GURL(source);
    return GURL();
  }

  std::string start_url_source;
  if (data_->GetManifest()->GetString(keys::kStartURLKey, &start_url_source) &&
      !start_url_source.empty()) {
    GURL url(start_url_source);
    if (url.is_valid() && url.SchemeIsHTTPOrHTTPS())
      return url;
    url = data_->GetResourceURL(start_url_source);
    if (url.is_valid())
      return url;
  }

  GURL url = GetAbsoluteURLFromKey(keys::kLaunchLocalPathKey);
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

GURL Application::GetStartURL(Manifest::Type type) const {
  switch (type) {
    case Manifest::Type::TYPE_WIDGET:
      return GetStartURL<Manifest::Type::TYPE_WIDGET>();
    case Manifest::Type::TYPE_MANIFEST:
      return GetStartURL<Manifest::Type::TYPE_MANIFEST>();
    default:
      NOTREACHED() << "Unknown manifest type";
      return GURL();
  }
}

template<>
void Application::SetWindowShowState<Manifest::TYPE_WIDGET>(
    NativeAppWindow::CreateParams* params) {
  const Manifest* manifest = data_->GetManifest();
  std::string view_modes_string;
  params->state = ui::SHOW_STATE_DEFAULT;
  if (manifest->GetString(widget_keys::kViewModesKey, &view_modes_string)) {
    // FIXME: ATM only 'fullscreen' and 'windowed' values are supported.
    // If the first user prefererence is 'fullscreen', set window show state
    // FULLSCREEN, otherwise set the default window show state.
    std::vector<std::string> modes = base::SplitString(
        view_modes_string, " ", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);
    if (!modes.empty() && modes[0] == "fullscreen")
      params->state = ui::SHOW_STATE_FULLSCREEN;
  }
}

template<>
void Application::SetWindowShowState<Manifest::TYPE_MANIFEST>(
    NativeAppWindow::CreateParams* params) {
  const Manifest* manifest = data_->GetManifest();
  std::string display_string;

  // FIXME: As we do not support browser mode, the default fallback will be
  // minimal-ui mode.
  params->display_mode = blink::WebDisplayModeMinimalUi;
  params->state = ui::SHOW_STATE_DEFAULT;
  if (!manifest->GetString(keys::kDisplay, &display_string))
    return;

  if (display_string == values::kDisplayModeFullscreen) {
    params->display_mode = blink::WebDisplayModeFullscreen;
    params->state = ui::SHOW_STATE_FULLSCREEN;
  } else if (display_string == values::kDisplayModeStandalone) {
    params->display_mode = blink::WebDisplayModeStandalone;
  }
}

bool Application::Launch() {
  if (!runtimes_.empty()) {
    LOG(ERROR) << "Attempt to launch app with id " << id()
               << ", but it is already running.";
    return false;
  }

  CHECK(!render_process_host_);

  GURL url = GetStartURL(data_->manifest_type());
  if (!url.is_valid())
    return false;

  auto site = content::SiteInstance::CreateForURL(browser_context_, url);
  Runtime* runtime = Runtime::Create(browser_context_, site);
  runtime->set_observer(this);
  runtimes_.push_back(runtime);
  render_process_host_ = runtime->GetRenderProcessHost();
  render_process_host_->AddObserver(this);
  if (security_policy_)
    security_policy_->EnforceForRenderer(render_process_host_);

  web_contents_ = runtime->web_contents();
  runtime->LoadURL(url);

  NativeAppWindow::CreateParams params;
  data_->manifest_type() == Manifest::TYPE_WIDGET ?
      SetWindowShowState<Manifest::TYPE_WIDGET>(&params) :
      SetWindowShowState<Manifest::TYPE_MANIFEST>(&params);

  params.bounds = data_->window_bounds();
  params.minimum_size.set_width(data_->window_min_size().width());
  params.minimum_size.set_height(data_->window_min_size().height());
  params.maximum_size.set_width(data_->window_max_size().width());
  params.maximum_size.set_height(data_->window_max_size().height());

  window_show_params_ = params;
  // Only the first runtime can have a launch screen.
  params.splash_screen_path = GetSplashScreenPath();
  runtime->set_ui_delegate(RuntimeUIDelegate::Create(runtime, params));

  // Set preferred orientation just before showing the UI.
  std::string orientation;
  if (data_->GetManifest()->GetString(keys::kOrientationKey, &orientation)
    && !orientation.empty())
    platform_util::SetPreferredScreenOrientation(orientation);

  runtime->Show();

  return true;
}

GURL Application::GetAbsoluteURLFromKey(const std::string& key) const {
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
  runtime->set_observer(this);
  runtime->set_ui_delegate(
      RuntimeUIDelegate::Create(runtime, window_show_params_));
  runtime->Show();
  runtimes_.push_back(runtime);
}

void Application::OnRuntimeClosed(Runtime* runtime) {
  auto found = std::find(runtimes_.begin(), runtimes_.end(), runtime);
  CHECK(found != runtimes_.end());
  runtimes_.erase(found);

  if (runtimes_.empty())
    base::MessageLoop::current()->PostTask(FROM_HERE,
        base::Bind(&Application::NotifyTermination,
                   weak_factory_.GetWeakPtr()));
}

void Application::OnApplicationExitRequested(Runtime* runtime) {
  Terminate();
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

bool Application::UseExtension(const std::string& extension_name) const {
  // TODO(Bai): Tells whether the application contains the specified extension
  return true;
}

bool Application::RegisterPermissions(const std::string& extension_name,
                                      const std::string& perm_table) {
  // TODO(Bai): Parse the permission table and fill in the name_perm_map_
  // The perm_table format is a simple JSON string, like
  // [{"permission_name":"echo","apis":["add","remove","get"]}]
  std::unique_ptr<base::Value> root = base::JSONReader().ReadToValue(perm_table);
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
        static_cast<base::DictionaryValue*>(iter->get());
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
