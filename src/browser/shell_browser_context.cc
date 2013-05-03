// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/browser/shell_browser_context.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/environment.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/threading/thread.h"
#include "cameo/src/browser/shell_download_manager_delegate.h"
#include "cameo/src/browser/shell_url_request_context_getter.h"
#include "cameo/src/common/shell_switches.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/content_switches.h"


#if defined(OS_WIN)
#include "base/base_paths_win.h"
#elif defined(OS_LINUX)
#include "base/nix/xdg_util.h"
#elif defined(OS_MACOSX)
#include "base/base_paths_mac.h"
#endif

namespace content {

class ShellBrowserContext::ShellResourceContext : public ResourceContext {
 public:
  ShellResourceContext() : getter_(NULL) {}
  virtual ~ShellResourceContext() {}

  // ResourceContext implementation:
  virtual net::HostResolver* GetHostResolver() OVERRIDE {
    CHECK(getter_);
    return getter_->host_resolver();
  }
  virtual net::URLRequestContext* GetRequestContext() OVERRIDE {
    CHECK(getter_);
    return getter_->GetURLRequestContext();
  }

  void set_url_request_context_getter(ShellURLRequestContextGetter* getter) {
    getter_ = getter;
  }

 private:
  ShellURLRequestContextGetter* getter_;

  DISALLOW_COPY_AND_ASSIGN(ShellResourceContext);
};

ShellBrowserContext::ShellBrowserContext(bool off_the_record)
    : off_the_record_(off_the_record),
      ignore_certificate_errors_(false),
      resource_context_(new ShellResourceContext) {
  InitWhileIOAllowed();
}

ShellBrowserContext::~ShellBrowserContext() {
  if (resource_context_.get()) {
    BrowserThread::DeleteSoon(
      BrowserThread::IO, FROM_HERE, resource_context_.release());
  }
}

void ShellBrowserContext::InitWhileIOAllowed() {
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kIgnoreCertificateErrors)) {
    ignore_certificate_errors_ = true;
  }
  if (cmd_line->HasSwitch(switches::kContentShellDataPath)) {
    path_ = cmd_line->GetSwitchValuePath(switches::kContentShellDataPath);
    return;
  }
#if defined(OS_WIN)
  CHECK(PathService::Get(base::DIR_LOCAL_APP_DATA, &path_));
  path_ = path_.Append(std::wstring(L"content_shell"));
#elif defined(OS_LINUX)
  scoped_ptr<base::Environment> env(base::Environment::Create());
  base::FilePath config_dir(
      base::nix::GetXDGDirectory(env.get(),
                                 base::nix::kXdgConfigHomeEnvVar,
                                 base::nix::kDotConfigDir));
  path_ = config_dir.Append("content_shell");
#elif defined(OS_MACOSX)
  CHECK(PathService::Get(base::DIR_APP_DATA, &path_));
  path_ = path_.Append("Chromium Content Shell");
#elif defined(OS_ANDROID)
  CHECK(PathService::Get(base::DIR_ANDROID_APP_DATA, &path_));
  path_ = path_.Append(FILE_PATH_LITERAL("content_shell"));
#else
  NOTIMPLEMENTED();
#endif

  if (!file_util::PathExists(path_))
    file_util::CreateDirectory(path_);
}

base::FilePath ShellBrowserContext::GetPath() {
  return path_;
}

bool ShellBrowserContext::IsOffTheRecord() const {
  return off_the_record_;
}

DownloadManagerDelegate* ShellBrowserContext::GetDownloadManagerDelegate()  {
  DownloadManager* manager = BrowserContext::GetDownloadManager(this);

  if (!download_manager_delegate_.get()) {
    download_manager_delegate_ = new ShellDownloadManagerDelegate();
    download_manager_delegate_->SetDownloadManager(manager);
  }

  return download_manager_delegate_.get();
}

net::URLRequestContextGetter* ShellBrowserContext::GetRequestContext()  {
  return GetDefaultStoragePartition(this)->GetURLRequestContext();
}

net::URLRequestContextGetter* ShellBrowserContext::CreateRequestContext(
    ProtocolHandlerMap* protocol_handlers) {
  DCHECK(!url_request_getter_);
  url_request_getter_ = new ShellURLRequestContextGetter(
      ignore_certificate_errors_,
      GetPath(),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE),
      protocol_handlers);
  resource_context_->set_url_request_context_getter(url_request_getter_.get());
  return url_request_getter_.get();
}

net::URLRequestContextGetter*
    ShellBrowserContext::GetRequestContextForRenderProcess(
        int renderer_child_id)  {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    ShellBrowserContext::GetMediaRequestContext()  {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    ShellBrowserContext::GetMediaRequestContextForRenderProcess(
        int renderer_child_id)  {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    ShellBrowserContext::GetMediaRequestContextForStoragePartition(
        const base::FilePath& partition_path,
        bool in_memory) {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    ShellBrowserContext::CreateRequestContextForStoragePartition(
        const base::FilePath& partition_path,
        bool in_memory,
        ProtocolHandlerMap* protocol_handlers) {
  return NULL;
}

ResourceContext* ShellBrowserContext::GetResourceContext()  {
  return resource_context_.get();
}

GeolocationPermissionContext*
    ShellBrowserContext::GetGeolocationPermissionContext()  {
  return NULL;
}

SpeechRecognitionPreferences*
    ShellBrowserContext::GetSpeechRecognitionPreferences() {
  return NULL;
}

quota::SpecialStoragePolicy* ShellBrowserContext::GetSpecialStoragePolicy() {
  return NULL;
}

}  // namespace content
