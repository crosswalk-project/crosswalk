// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_context.h"

#include <string>
#include <utility>
#include <vector>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
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
#include "xwalk/runtime/browser/runtime_geolocation_permission_context.h"
#include "xwalk/runtime/browser/runtime_url_request_context_getter.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#include "xwalk/runtime/common/xwalk_switches.h"

#if defined(OS_ANDROID)
#include "base/strings/string_split.h"
#endif

using content::BrowserThread;
using content::DownloadManager;

namespace xwalk {

class RuntimeContext::RuntimeResourceContext : public content::ResourceContext {
 public:
  RuntimeResourceContext() : getter_(NULL) {}
  virtual ~RuntimeResourceContext() {}

  // ResourceContext implementation:
  virtual net::HostResolver* GetHostResolver() OVERRIDE {
    CHECK(getter_);
    return getter_->host_resolver();
  }
  virtual net::URLRequestContext* GetRequestContext() OVERRIDE {
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

RuntimeContext::RuntimeContext()
  : resource_context_(new RuntimeResourceContext) {
  InitWhileIOAllowed();
#if defined(OS_ANDROID)
  InitVisitedLinkMaster();
#endif
}

RuntimeContext::~RuntimeContext() {
  if (resource_context_.get()) {
    BrowserThread::DeleteSoon(
        BrowserThread::IO, FROM_HERE, resource_context_.release());
  }
}

// static
RuntimeContext* RuntimeContext::FromWebContents(
    content::WebContents* web_contents) {
  // This is safe; this is the only implementation of the browser context.
  return static_cast<RuntimeContext*>(web_contents->GetBrowserContext());
}

void RuntimeContext::InitWhileIOAllowed() {
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kXWalkDataPath)) {
    base::FilePath path =
        cmd_line->GetSwitchValuePath(switches::kXWalkDataPath);
    PathService::OverrideAndCreateIfNeeded(
        xwalk::DIR_DATA_PATH, path, false, true);
  }
}

base::FilePath RuntimeContext::GetPath() const {
  base::FilePath result;
#if defined(OS_ANDROID)
  CHECK(PathService::Get(base::DIR_ANDROID_APP_DATA, &result));
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kXWalkProfileName))
    result = result.Append(
        cmd_line->GetSwitchValuePath(switches::kXWalkProfileName));
#else
  CHECK(PathService::Get(xwalk::DIR_DATA_PATH, &result));
#endif
  return result;
}

bool RuntimeContext::IsOffTheRecord() const {
  // We don't consider off the record scenario.
  return false;
}

content::DownloadManagerDelegate* RuntimeContext::GetDownloadManagerDelegate() {
  content::DownloadManager* manager = BrowserContext::GetDownloadManager(this);

  if (!download_manager_delegate_.get()) {
    download_manager_delegate_ = new RuntimeDownloadManagerDelegate();
    download_manager_delegate_->SetDownloadManager(manager);
  }

  return download_manager_delegate_.get();
}

net::URLRequestContextGetter* RuntimeContext::GetRequestContext() {
  return GetDefaultStoragePartition(this)->GetURLRequestContext();
}

net::URLRequestContextGetter*
    RuntimeContext::GetRequestContextForRenderProcess(
        int renderer_child_id) {
#if defined(OS_ANDROID)
  return GetRequestContext();
#else
  content::RenderProcessHost* rph =
      content::RenderProcessHost::FromID(renderer_child_id);
  return rph->GetStoragePartition()->GetURLRequestContext();
#endif
}

net::URLRequestContextGetter* RuntimeContext::GetMediaRequestContext() {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    RuntimeContext::GetMediaRequestContextForRenderProcess(
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
    RuntimeContext::GetMediaRequestContextForStoragePartition(
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

content::ResourceContext* RuntimeContext::GetResourceContext()  {
  return resource_context_.get();
}

content::BrowserPluginGuestManager*
RuntimeContext::GetGuestManager() {
  return NULL;
}

storage::SpecialStoragePolicy* RuntimeContext::GetSpecialStoragePolicy() {
  return NULL;
}

content::PushMessagingService* RuntimeContext::GetPushMessagingService() {
  return NULL;
}

content::SSLHostStateDelegate* RuntimeContext::GetSSLHostStateDelegate() {
  return NULL;
}

RuntimeURLRequestContextGetter* RuntimeContext::GetURLRequestContextGetterById(
    const std::string& pkg_id) {
  for (PartitionPathContextGetterMap::iterator it = context_getters_.begin();
       it != context_getters_.end(); ++it) {
    if (it->first.find(pkg_id))
      return it->second.get();
  }
  return 0;
}

net::URLRequestContextGetter* RuntimeContext::CreateRequestContext(
    content::ProtocolHandlerMap* protocol_handlers,
    content::URLRequestInterceptorScopedVector request_interceptors) {
  DCHECK(!url_request_getter_.get());

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
  RuntimeContext::CreateRequestContextForStoragePartition(
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
  return context_getter.get();
#endif
}

#if defined(OS_ANDROID)
void RuntimeContext::SetCSPString(const std::string& csp) {
  // Check format of csp string.
  std::vector<std::string> policies;
  base::SplitString(csp, ';', &policies);
  for (size_t i = 0; i < policies.size(); ++i) {
    size_t found = policies[i].find(' ');
    if (found == std::string::npos) {
      LOG(INFO) << "Invalid value of directive: " << policies[i];
      return;
    }
  }
  csp_ = csp;
}

std::string RuntimeContext::GetCSPString() const {
  return csp_;
}

void RuntimeContext::InitVisitedLinkMaster() {
  visitedlink_master_.reset(
      new visitedlink::VisitedLinkMaster(this, this, false));
  visitedlink_master_->Init();
}

void RuntimeContext::AddVisitedURLs(const std::vector<GURL>& urls) {
  DCHECK(visitedlink_master_.get());
  visitedlink_master_->AddURLs(urls);
}

void RuntimeContext::RebuildTable(
    const scoped_refptr<URLEnumerator>& enumerator) {
  // XWalkView rebuilds from XWalkWebChromeClient.getVisitedHistory. The client
  // can change in the lifetime of this XWalkView and may not yet be set here.
  // Therefore this initialization path is not used.
  enumerator->OnComplete(true);
}
#endif

}  // namespace xwalk
