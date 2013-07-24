// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_context.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "xwalk/runtime/browser/runtime_download_manager_delegate.h"
#include "xwalk/runtime/browser/runtime_url_request_context_getter.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"

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

#if !defined(OS_ANDROID)
RuntimeContext::RuntimeContext()
  : resource_context_(new RuntimeResourceContext) {
#else
RuntimeContext::RuntimeContext() {
#endif
  InitWhileIOAllowed();
}

RuntimeContext::~RuntimeContext() {
  if (resource_context_) {
    BrowserThread::DeleteSoon(
        BrowserThread::IO, FROM_HERE, resource_context_.release());
  }
}

#if defined(OS_ANDROID)
// static
RuntimeContext* RuntimeContext::FromWebContents(
    content::WebContents* web_contents) {
  // This is safe; this is the only implementation of the browser context.
  return static_cast<RuntimeContext*>(web_contents->GetBrowserContext());
}
#endif

void RuntimeContext::InitWhileIOAllowed() {
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kXWalkDataPath)) {
    base::FilePath path =
        cmd_line->GetSwitchValuePath(switches::kXWalkDataPath);
    PathService::OverrideAndCreateIfNeeded(xwalk::DIR_DATA_PATH, path, true);
  }
}

base::FilePath RuntimeContext::GetPath() {
  base::FilePath result;
#if defined(OS_ANDROID)
  CHECK(PathService::Get(base::DIR_ANDROID_APP_DATA, &result));
#else
  CHECK(PathService::Get(xwalk::DIR_DATA_PATH, &result));
#endif
  return result;
}

#if defined(OS_ANDROID)
void RuntimeContext::InitializeBeforeThreadCreation() {
}

void RuntimeContext::PreMainMessageLoopRun() {
}
#endif

bool RuntimeContext::IsOffTheRecord() const {
  // We don't consider off the record scenario.
  return false;
}

content::DownloadManagerDelegate* RuntimeContext::GetDownloadManagerDelegate() {
#if defined(OS_ANDROID)
  return NULL;
#else
  content::DownloadManager* manager = BrowserContext::GetDownloadManager(this);

  if (!download_manager_delegate_) {
    download_manager_delegate_ = new RuntimeDownloadManagerDelegate();
    download_manager_delegate_->SetDownloadManager(manager);
  }

  return download_manager_delegate_.get();
#endif
}

net::URLRequestContextGetter* RuntimeContext::GetRequestContext() {
  return GetDefaultStoragePartition(this)->GetURLRequestContext();
}

net::URLRequestContextGetter*
    RuntimeContext::GetRequestContextForRenderProcess(
        int renderer_child_id)  {
  return GetRequestContext();
}

net::URLRequestContextGetter* RuntimeContext::GetMediaRequestContext()  {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    RuntimeContext::GetMediaRequestContextForRenderProcess(
        int renderer_child_id)  {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    RuntimeContext::GetMediaRequestContextForStoragePartition(
        const base::FilePath& partition_path,
        bool in_memory) {
  return GetRequestContext();
}

content::ResourceContext* RuntimeContext::GetResourceContext()  {
#if defined(OS_ANDROID)
  if (!resource_context_.get())
    resource_context_.reset(new RuntimeResourceContext);
#endif
  return resource_context_.get();
}

content::GeolocationPermissionContext*
    RuntimeContext::GetGeolocationPermissionContext()  {
  return NULL;
}

content::SpeechRecognitionPreferences*
    RuntimeContext::GetSpeechRecognitionPreferences() {
  return NULL;
}

quota::SpecialStoragePolicy* RuntimeContext::GetSpecialStoragePolicy() {
  return NULL;
}

net::URLRequestContextGetter* RuntimeContext::CreateRequestContext(
    content::ProtocolHandlerMap* protocol_handlers) {
  DCHECK(!url_request_getter_);
  url_request_getter_ = new RuntimeURLRequestContextGetter(
      false, /* ignore_certificate_error = false */
      GetPath(),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE),
      protocol_handlers);
  resource_context_->set_url_request_context_getter(url_request_getter_.get());
  return url_request_getter_.get();
}

net::URLRequestContextGetter*
    RuntimeContext::CreateRequestContextForStoragePartition(
        const base::FilePath& partition_path,
        bool in_memory,
        content::ProtocolHandlerMap* protocol_handlers) {
  return NULL;
}

}  // namespace xwalk
