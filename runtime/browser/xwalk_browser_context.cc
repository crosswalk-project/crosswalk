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
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/pref_service.h"
#include "base/prefs/pref_service_factory.h"
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
#include "xwalk/runtime/browser/xwalk_permission_manager.h"
#include "xwalk/runtime/browser/xwalk_pref_store.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#include "xwalk/runtime/common/xwalk_switches.h"

#if defined(OS_ANDROID)
#include "base/strings/string_split.h"
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
  : resource_context_(new RuntimeResourceContext) {
  InitWhileIOAllowed();
#if defined(OS_ANDROID)
  InitVisitedLinkMaster();
  InitFormDatabaseService();
#endif
  CHECK(!g_browser_context);
  g_browser_context = this;
}

XWalkBrowserContext::~XWalkBrowserContext() {
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
  if (cmd_line->HasSwitch(switches::kXWalkDataPath)) {
    base::FilePath path =
        cmd_line->GetSwitchValuePath(switches::kXWalkDataPath);
    PathService::OverrideAndCreateIfNeeded(
        DIR_DATA_PATH, path, false, true);
  }
}

scoped_ptr<content::ZoomLevelDelegate>
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

net::URLRequestContextGetter* XWalkBrowserContext::GetRequestContext() {
  return GetDefaultStoragePartition(this)->GetURLRequestContext();
}

net::URLRequestContextGetter*
    XWalkBrowserContext::GetRequestContextForRenderProcess(
        int renderer_child_id) {
#if defined(OS_ANDROID)
  return GetRequestContext();
#else
  content::RenderProcessHost* rph =
      content::RenderProcessHost::FromID(renderer_child_id);
  return rph->GetStoragePartition()->GetURLRequestContext();
#endif
}

net::URLRequestContextGetter* XWalkBrowserContext::GetMediaRequestContext() {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    XWalkBrowserContext::GetMediaRequestContextForRenderProcess(
        int renderer_child_id) {
#if defined(OS_ANDROID)
  return GetRequestContext();
#else
  content::RenderProcessHost* rph =
      content::RenderProcessHost::FromID(renderer_child_id);
  return rph->GetStoragePartition()->GetURLRequestContext();
#endif
}

net::URLRequestContextGetter*
    XWalkBrowserContext::GetMediaRequestContextForStoragePartition(
        const base::FilePath& partition_path,
        bool in_memory) {
#if defined(OS_ANDROID)
  return GetRequestContext();
#else
  PartitionPathContextGetterMap::iterator iter =
      context_getters_.find(partition_path.value());
  CHECK(iter != context_getters_.end());
  return iter->second.get();
#endif
}

content::ResourceContext* XWalkBrowserContext::GetResourceContext()  {
  return resource_context_.get();
}

content::BrowserPluginGuestManager*
XWalkBrowserContext::GetGuestManager() {
  return NULL;
}

storage::SpecialStoragePolicy* XWalkBrowserContext::GetSpecialStoragePolicy() {
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
    permission_manager_.reset(new XWalkPermissionManager());
  return permission_manager_.get();
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

  application::ApplicationService* service =
      XWalkRunner::GetInstance()->app_system()->application_service();
  protocol_handlers->insert(std::pair<std::string,
        linked_ptr<net::URLRequestJobFactory::ProtocolHandler> >(
          application::kApplicationScheme,
          application::CreateApplicationProtocolHandler(service)));

  url_request_getter_ = new RuntimeURLRequestContextGetter(
      false, /* ignore_certificate_error = false */
      GetPath(),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE),
      protocol_handlers, request_interceptors.Pass());
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
      protocol_handlers, request_interceptors.Pass());

  context_getters_.insert(
      std::make_pair(partition_path.value(), context_getter));
  // Make sure that the default url request getter has been initialized,
  // please refer to https://crosswalk-project.org/jira/browse/XWALK-2890
  // for more details.
  if (!url_request_getter_)
    CreateRequestContext(protocol_handlers, request_interceptors.Pass());

  return context_getter.get();
#endif
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

void XWalkBrowserContext::InitFormDatabaseService() {
  base::FilePath user_data_dir;
  CHECK(PathService::Get(base::DIR_ANDROID_APP_DATA, &user_data_dir));
  form_database_service_.reset(new XWalkFormDatabaseService(user_data_dir));
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
  pref_registry->RegisterDoublePref(
      autofill::prefs::kAutofillPositiveUploadRate, 0.0);
  pref_registry->RegisterDoublePref(
      autofill::prefs::kAutofillNegativeUploadRate, 0.0);

  base::PrefServiceFactory pref_service_factory;
  pref_service_factory.set_user_prefs(make_scoped_refptr(new XWalkPrefStore()));
  pref_service_factory.set_read_error_callback(base::Bind(&HandleReadError));
  user_pref_service_ = pref_service_factory.Create(pref_registry).Pass();

  user_prefs::UserPrefs::Set(this, user_pref_service_.get());
}

void XWalkBrowserContext::UpdateAcceptLanguages(
    const std::string& accept_languages) {
  RuntimeURLRequestContextGetter* url_request_context_getter =
      url_request_getter_.get();
  if (!url_request_context_getter)
    return;
  url_request_context_getter->UpdateAcceptLanguages(accept_languages);
}
#endif

}  // namespace xwalk
