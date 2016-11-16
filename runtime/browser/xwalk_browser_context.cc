// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_browser_context.h"

#include <string>
#include <utility>
#include <vector>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_service_factory.h"
#include "base/strings/utf_string_conversions.h"
#include "components/autofill/core/common/autofill_pref_names.h"
#include "components/user_prefs/user_prefs.h"
#include "components/visitedlink/browser/visitedlink_master.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_protocols.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/runtime/browser/runtime_download_manager_delegate.h"
#include "xwalk/runtime/browser/runtime_url_request_context_getter.h"
#include "xwalk/runtime/browser/xwalk_content_settings.h"
#include "xwalk/runtime/browser/xwalk_permission_manager.h"
#include "xwalk/runtime/browser/xwalk_pref_store.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/browser/xwalk_special_storage_policy.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#include "xwalk/runtime/common/xwalk_switches.h"


#if defined(OS_ANDROID)
#include "base/strings/string_split.h"
#elif defined(OS_WIN)
#include "base/base_paths_win.h"
#elif defined(OS_LINUX)
#include "base/nix/xdg_util.h"
#elif defined(OS_MACOSX)
#include "base/base_paths_mac.h"
#endif

using content::BrowserThread;
using content::DownloadManager;

namespace xwalk {

class XWalkBrowserContext::RuntimeResourceContext :
    public content::ResourceContext {
 public:
  RuntimeResourceContext() : getter_(NULL) {}
  ~RuntimeResourceContext() override {}

  // ResourceContext implementation:
  net::HostResolver* GetHostResolver() override {
    CHECK(getter_);
    return getter_->host_resolver();
  }
  net::URLRequestContext* GetRequestContext() override {
    CHECK(getter_);
    return getter_->GetURLRequestContext();
  }

  void set_url_request_context_getter(RuntimeURLRequestContextGetter* getter) {
    getter_ = getter;
  }

 private:
  RuntimeURLRequestContextGetter* getter_;

  DISALLOW_COPY_AND_ASSIGN(RuntimeResourceContext);
};

XWalkBrowserContext* g_browser_context = nullptr;

void HandleReadError(PersistentPrefStore::PrefReadError error) {
  LOG(ERROR) << "Failed to read preference, error num: " << error;
}

XWalkBrowserContext::XWalkBrowserContext()
    : resource_context_(new RuntimeResourceContext),
    save_form_data_(true) {
  InitWhileIOAllowed();
  InitFormDatabaseService();
  InitVisitedLinkMaster();
  CHECK(!g_browser_context);
  g_browser_context = this;
}

XWalkBrowserContext::~XWalkBrowserContext() {
#if !defined(OS_ANDROID)
  XWalkContentSettings::GetInstance()->Shutdown();
#endif
  if (resource_context_.get()) {
    BrowserThread::DeleteSoon(
        BrowserThread::IO, FROM_HERE, resource_context_.release());
  }
  DCHECK_EQ(this, g_browser_context);
  g_browser_context = nullptr;
}

// static
XWalkBrowserContext* XWalkBrowserContext::GetDefault() {
  // TODO(joth): rather than store in a global here, lookup this instance
  // from the Java-side peer.
  return g_browser_context;
}

// static
XWalkBrowserContext* XWalkBrowserContext::FromWebContents(
    content::WebContents* web_contents) {
  // This is safe; this is the only implementation of the browser context.
  return static_cast<XWalkBrowserContext*>(
      web_contents->GetBrowserContext());
}

void XWalkBrowserContext::InitWhileIOAllowed() {
  base::CommandLine* cmd_line = base::CommandLine::ForCurrentProcess();
  base::FilePath path;
  if (cmd_line->HasSwitch(switches::kXWalkDataPath)) {
    path = cmd_line->GetSwitchValuePath(switches::kXWalkDataPath);
    PathService::OverrideAndCreateIfNeeded(
        DIR_DATA_PATH, path, false, true);
    BrowserContext::Initialize(this, path);
  } else {
    base::FilePath::StringType xwalk_suffix;
    xwalk_suffix = FILE_PATH_LITERAL("xwalk");
#if defined(OS_WIN)
    CHECK(PathService::Get(base::DIR_LOCAL_APP_DATA, &path));
    path = path.Append(xwalk_suffix);
#elif defined(OS_LINUX)
    std::unique_ptr<base::Environment> env(base::Environment::Create());
    base::FilePath config_dir(
        base::nix::GetXDGDirectory(env.get(),
                                   base::nix::kXdgConfigHomeEnvVar,
                                   base::nix::kDotConfigDir));
    path = config_dir.Append(xwalk_suffix);
#elif defined(OS_MACOSX)
    CHECK(PathService::Get(base::DIR_APP_DATA, &path));
    path = path.Append(xwalk_suffix);
#elif defined(OS_ANDROID)
    CHECK(PathService::Get(base::DIR_ANDROID_APP_DATA, &path));
    path = path.Append(xwalk_suffix);
#else
    NOTIMPLEMENTED();
#endif
  }

  BrowserContext::Initialize(this, path);
#if !defined(OS_ANDROID)
  XWalkContentSettings::GetInstance()->Init();
#endif
}

std::unique_ptr<content::ZoomLevelDelegate>
XWalkBrowserContext::CreateZoomLevelDelegate(
    const base::FilePath& partition_path) {
  return nullptr;
}

base::FilePath XWalkBrowserContext::GetPath() const {
  base::FilePath result;
#if defined(OS_ANDROID)
  base::CommandLine* cmd_line = base::CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kUserDataDir))
    result = cmd_line->GetSwitchValuePath(switches::kUserDataDir);
  if (result.empty())
    CHECK(PathService::Get(base::DIR_ANDROID_APP_DATA, &result));
  if (cmd_line->HasSwitch(switches::kXWalkProfileName))
    result = result.Append(
        cmd_line->GetSwitchValuePath(switches::kXWalkProfileName));
#else
  CHECK(PathService::Get(DIR_DATA_PATH, &result));
#endif
  return result;
}

bool XWalkBrowserContext::IsOffTheRecord() const {
  // We don't consider off the record scenario.
  return false;
}

content::DownloadManagerDelegate*
XWalkBrowserContext::GetDownloadManagerDelegate() {
  content::DownloadManager* manager = BrowserContext::GetDownloadManager(this);

  if (!download_manager_delegate_.get()) {
    download_manager_delegate_ = new RuntimeDownloadManagerDelegate();
    download_manager_delegate_->SetDownloadManager(manager);
  }

  return download_manager_delegate_.get();
}

content::ResourceContext* XWalkBrowserContext::GetResourceContext()  {
  return resource_context_.get();
}

content::BrowserPluginGuestManager*
XWalkBrowserContext::GetGuestManager() {
  return NULL;
}

storage::SpecialStoragePolicy* XWalkBrowserContext::GetSpecialStoragePolicy() {
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kUnlimitedStorage)) {
    if (!special_storage_policy_.get())
      special_storage_policy_ = new XWalkSpecialStoragePolicy();
    return special_storage_policy_.get();
  }
  return NULL;
}

content::PushMessagingService* XWalkBrowserContext::GetPushMessagingService() {
  return NULL;
}

content::SSLHostStateDelegate* XWalkBrowserContext::GetSSLHostStateDelegate() {
  if (!ssl_host_state_delegate_.get()) {
    ssl_host_state_delegate_.reset(new XWalkSSLHostStateDelegate());
  }
  return ssl_host_state_delegate_.get();
}

content::PermissionManager* XWalkBrowserContext::GetPermissionManager() {
  if (!permission_manager_.get())
    permission_manager_.reset(new XWalkPermissionManager(application_service_));
  return permission_manager_.get();
}

content::BackgroundSyncController*
XWalkBrowserContext::GetBackgroundSyncController() {
  return nullptr;
}

RuntimeURLRequestContextGetter*
XWalkBrowserContext::GetURLRequestContextGetterById(
    const std::string& pkg_id) {
  for (PartitionPathContextGetterMap::iterator it = context_getters_.begin();
       it != context_getters_.end(); ++it) {
#if defined(OS_WIN)
    if (it->first.find(base::UTF8ToWide(pkg_id)))
#else
    if (it->first.find(pkg_id))
#endif
      return it->second.get();
  }
  return 0;
}

net::URLRequestContextGetter* XWalkBrowserContext::CreateRequestContext(
    content::ProtocolHandlerMap* protocol_handlers,
    content::URLRequestInterceptorScopedVector request_interceptors) {
  if (url_request_getter_)
    return url_request_getter_.get();

  protocol_handlers->insert(std::pair<std::string,
        linked_ptr<net::URLRequestJobFactory::ProtocolHandler> >(
          application::kApplicationScheme,
          application::CreateApplicationProtocolHandler(
              application_service_)));

  url_request_getter_ = new RuntimeURLRequestContextGetter(
      false, /* ignore_certificate_error = false */
      GetPath(),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE),
      protocol_handlers, std::move(request_interceptors));
  resource_context_->set_url_request_context_getter(url_request_getter_.get());
  return url_request_getter_.get();
}

net::URLRequestContextGetter*
  XWalkBrowserContext::CreateRequestContextForStoragePartition(
      const base::FilePath& partition_path,
      bool in_memory,
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector request_interceptors) {
#if defined(OS_ANDROID)
    return NULL;
#else
  PartitionPathContextGetterMap::iterator iter =
    context_getters_.find(partition_path.value());
  if (iter != context_getters_.end())
    return iter->second.get();

  application::ApplicationService* service =
      XWalkRunner::GetInstance()->app_system()->application_service();
  protocol_handlers->insert(std::pair<std::string,
        linked_ptr<net::URLRequestJobFactory::ProtocolHandler> >(
          application::kApplicationScheme,
          application::CreateApplicationProtocolHandler(service)));

  scoped_refptr<RuntimeURLRequestContextGetter>
  context_getter = new RuntimeURLRequestContextGetter(
      false, /* ignore_certificate_error = false */
      partition_path,
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE),
      protocol_handlers, std::move(request_interceptors));

  context_getters_.insert(
      std::make_pair(partition_path.value(), context_getter));
  // Make sure that the default url request getter has been initialized,
  // please refer to https://crosswalk-project.org/jira/browse/XWALK-2890
  // for more details.
  if (!url_request_getter_)
    CreateRequestContext(protocol_handlers, std::move(request_interceptors));

  return context_getter.get();
#endif
}

net::URLRequestContextGetter* XWalkBrowserContext::CreateMediaRequestContext() {
  return url_request_getter_.get();
}

net::URLRequestContextGetter*
XWalkBrowserContext::CreateMediaRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory) {
#if defined(OS_ANDROID)
  return url_request_getter_.get();
#else
  PartitionPathContextGetterMap::iterator iter =
      context_getters_.find(partition_path.value());
  CHECK(iter != context_getters_.end());
  return iter->second.get();
#endif
}

XWalkFormDatabaseService* XWalkBrowserContext::GetFormDatabaseService() {
  return form_database_service_.get();
}

// Create user pref service for autofill functionality.
void XWalkBrowserContext::CreateUserPrefServiceIfNecessary() {
  if (user_pref_service_) return;

  PrefRegistrySimple* pref_registry = new PrefRegistrySimple();
  pref_registry->RegisterStringPref("intl.accept_languages", "");

  // We only use the autocomplete feature of the Autofill, which is
  // controlled via the manager_delegate. We don't use the rest
  // of autofill, which is why it is hardcoded as disabled here.
  pref_registry->RegisterBooleanPref(
    autofill::prefs::kAutofillEnabled, false);

  PrefServiceFactory pref_service_factory;
  pref_service_factory.set_user_prefs(make_scoped_refptr(new XWalkPrefStore()));
  pref_service_factory.set_read_error_callback(base::Bind(&HandleReadError));
  user_pref_service_ = pref_service_factory.Create(pref_registry);

  user_prefs::UserPrefs::Set(this, user_pref_service_.get());
}

void XWalkBrowserContext::UpdateAcceptLanguages(
    const std::string& accept_languages) {
  if (url_request_getter_)
    url_request_getter_->UpdateAcceptLanguages(accept_languages);
}

void XWalkBrowserContext::InitFormDatabaseService() {
  base::FilePath user_data_dir;
#if defined(OS_ANDROID)
  CHECK(PathService::Get(base::DIR_ANDROID_APP_DATA, &user_data_dir));
#elif defined(OS_WIN)
  CHECK(PathService::Get(base::DIR_APP_DATA, &user_data_dir));
#endif
  form_database_service_.reset(new XWalkFormDatabaseService(user_data_dir));
}

#if defined(OS_ANDROID)
void XWalkBrowserContext::SetCSPString(const std::string& csp) {
  // Check format of csp string.
  std::vector<std::string> policies = base::SplitString(
      csp, ";", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);
  for (size_t i = 0; i < policies.size(); ++i) {
    size_t found = policies[i].find(' ');
    if (found == std::string::npos) {
      LOG(INFO) << "Invalid value of directive: " << policies[i];
      return;
    }
  }
  csp_ = csp;
}

std::string XWalkBrowserContext::GetCSPString() const {
  return csp_;
}
#endif

void XWalkBrowserContext::InitVisitedLinkMaster() {
  visitedlink_master_.reset(
      new visitedlink::VisitedLinkMaster(this, this, false));
  visitedlink_master_->Init();
}

void XWalkBrowserContext::AddVisitedURLs(const std::vector<GURL>& urls) {
  DCHECK(visitedlink_master_.get());
  visitedlink_master_->AddURLs(urls);
}

void XWalkBrowserContext::RebuildTable(
    const scoped_refptr<URLEnumerator>& enumerator) {
  // XWalkView rebuilds from XWalkWebChromeClient.getVisitedHistory. The client
  // can change in the lifetime of this XWalkView and may not yet be set here.
  // Therefore this initialization path is not used.
  enumerator->OnComplete(true);
}

}  // namespace xwalk
