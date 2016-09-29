// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_protocols.h"

#include <algorithm>
#include <map>
#include <list>
#include <string>
#include <utility>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "base/numerics/safe_math.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"
#include "base/threading/thread_restrictions.h"
#include "base/threading/worker_pool.h"
#include "base/threading/sequenced_worker_pool.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/resource_request_info.h"
#include "url/url_util.h"
#include "net/base/net_errors.h"
#include "net/http/http_response_headers.h"
#include "net/http/http_response_info.h"
#include "net/url_request/url_request_error_job.h"
#include "net/url_request/url_request_file_job.h"
#include "net/url_request/url_request_simple_job.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/application_resource.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/manifest_handlers/csp_handler.h"
#include "xwalk/runtime/common/xwalk_system_locale.h"

using content::BrowserThread;
using content::ResourceRequestInfo;

namespace xwalk {

namespace keys = application_manifest_keys;

namespace application {

namespace {

net::HttpResponseHeaders* BuildHttpHeaders(
    const std::string& content_security_policy,
    const std::string& mime_type, const std::string& method,
    const base::FilePath& file_path, const base::FilePath& relative_path) {
  std::string raw_headers;
  if (method == "GET") {
    if (relative_path.empty())
      raw_headers.append("HTTP/1.1 400 Bad Request");
    else if (file_path.empty())
      raw_headers.append("HTTP/1.1 404 Not Found");
    else
      raw_headers.append("HTTP/1.1 200 OK");
  } else {
    raw_headers.append("HTTP/1.1 501 Not Implemented");
  }

  if (!content_security_policy.empty()) {
    raw_headers.append(1, '\0');
    raw_headers.append("Content-Security-Policy: ");
    raw_headers.append(content_security_policy);
  }

  raw_headers.append(1, '\0');
  raw_headers.append("Access-Control-Allow-Origin: *");

  if (!mime_type.empty()) {
    raw_headers.append(1, '\0');
    raw_headers.append("Content-Type: ");
    raw_headers.append(mime_type);
  }

  raw_headers.append(2, '\0');
  return new net::HttpResponseHeaders(raw_headers);
}

void ReadResourceFilePath(
    const ApplicationResource& resource,
    base::FilePath* file_path) {
  *file_path = resource.GetFilePath();
}

class URLRequestApplicationJob : public net::URLRequestFileJob {
 public:
  URLRequestApplicationJob(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate,
      const scoped_refptr<base::TaskRunner>& file_task_runner,
      const std::string& application_id,
      const base::FilePath& directory_path,
      const base::FilePath& relative_path,
      const std::string& content_security_policy,
      const std::list<std::string>& locales)
      : net::URLRequestFileJob(
          request, network_delegate, base::FilePath(), file_task_runner),
        content_security_policy_(content_security_policy),
        locales_(locales),
        resource_(application_id, directory_path, relative_path),
        relative_path_(relative_path),
        weak_factory_(this) {
  }

  void GetResponseInfo(net::HttpResponseInfo* info) override {
    std::string mime_type;
    GetMimeType(&mime_type);
    std::string method = request()->method();
    response_info_.headers = BuildHttpHeaders(
        content_security_policy_, mime_type, method, file_path_,
        relative_path_);
    *info = response_info_;
  }

  void Start() override {
    base::FilePath* read_file_path = new base::FilePath;

    resource_.SetLocales(locales_);
    bool posted = base::WorkerPool::PostTaskAndReply(
        FROM_HERE,
        base::Bind(&ReadResourceFilePath, resource_,
                   base::Unretained(read_file_path)),
        base::Bind(&URLRequestApplicationJob::OnFilePathRead,
                   weak_factory_.GetWeakPtr(),
                   base::Owned(read_file_path)),
        true /* task is slow */);
    DCHECK(posted);
  }

 protected:
  ~URLRequestApplicationJob() override {}

  std::string content_security_policy_;
  std::list<std::string> locales_;
  ApplicationResource resource_;
  base::FilePath relative_path_;

 private:
  void OnFilePathRead(base::FilePath* read_file_path) {
    file_path_ = *read_file_path;
    if (file_path_.empty())
      NotifyHeadersComplete();
    else
      URLRequestFileJob::Start();
  }

  net::HttpResponseInfo response_info_;
  base::WeakPtrFactory<URLRequestApplicationJob> weak_factory_;
};

// This class is a thread-safe cache of active application's data.
// This class is used by ApplicationProtocolHandler which lives on
// IO thread and hence cannot access ApplicationService directly.
class ApplicationDataCache : public ApplicationService::Observer {
 public:
  scoped_refptr<ApplicationData> GetApplicationData(
      const std::string& application_id) const {
    base::AutoLock lock(lock_);
    ApplicationData::ApplicationDataMap::const_iterator it =
        cache_.find(application_id);
    if (it != cache_.end()) {
      return it->second;
    }
    return NULL;
  }

  static void CreateIfNeeded(ApplicationService* service) {
    DCHECK(service);
    DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
    if (s_instance_)
      return;
    // The cache lives longer than ApplicationService,
    // so we do not need to remove cache_ from ApplicationService
    // observers list.
    s_instance_ = new ApplicationDataCache();
    service->AddObserver(s_instance_);
  }

  static ApplicationDataCache* Get() { return s_instance_;}

 private:
  void DidLaunchApplication(Application* app) override {
    base::AutoLock lock(lock_);
    cache_.insert(std::pair<std::string, scoped_refptr<ApplicationData> >(
        app->id(), app->data()));
  }

  void WillDestroyApplication(Application* app) override {
    base::AutoLock lock(lock_);
    cache_.erase(app->id());
  }

  ApplicationDataCache() = default;
  // The life time of the cache instance is equal to the process life time,
  // it is not supposed to be explicitly destroyed.
  ~ApplicationDataCache() override = default;

  ApplicationData::ApplicationDataMap cache_;
  mutable base::Lock lock_;

  static ApplicationDataCache* s_instance_;
};

ApplicationDataCache* ApplicationDataCache::s_instance_;

class ApplicationProtocolHandler
    : public net::URLRequestJobFactory::ProtocolHandler {
 public:
  explicit ApplicationProtocolHandler(ApplicationService* service) {
    ApplicationDataCache::CreateIfNeeded(service);
  }

  ~ApplicationProtocolHandler() override {}

  net::URLRequestJob* MaybeCreateJob(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(ApplicationProtocolHandler);
};

// The |locale| should be expanded to user agent locale.
// Such as, "en-us" will be expaned as "en-us, en".
void GetUserAgentLocales(const std::string& sys_locale,
                         std::list<std::string>& ua_locales) {  // NOLINT
  if (sys_locale.empty())
    return;

  std::string locale = base::ToLowerASCII(sys_locale);
  size_t position;
  do {
    ua_locales.push_back(locale);
    position = locale.find_last_of("-");
    locale = locale.substr(0, position);
  } while (position != std::string::npos);
}

namespace {
const char kSpace[] = " ";
const char kSemicolon[] = ";";
}  // namespace

net::URLRequestJob*
ApplicationProtocolHandler::MaybeCreateJob(
    net::URLRequest* request, net::NetworkDelegate* network_delegate) const {
  const std::string& application_id = request->url().host();
  scoped_refptr<ApplicationData> application =
      ApplicationDataCache::Get()->GetApplicationData(application_id);

  if (!application.get())
    return new net::URLRequestErrorJob(
        request, network_delegate, net::ERR_FILE_NOT_FOUND);

  base::FilePath relative_path =
      ApplicationURLToRelativeFilePath(request->url());
  base::FilePath directory_path = application->path();
  std::string content_security_policy;
  const char* csp_key = GetCSPKey(application->manifest_type());
  const CSPInfo* csp_info =
      static_cast<CSPInfo*>(application->GetManifestData(csp_key));
  if (csp_info) {
    for (auto& directive : csp_info->GetDirectives()) {
      content_security_policy.append(directive.first)
          .append(kSpace)
          .append(base::JoinString(directive.second, kSpace))
          .append(kSemicolon);
    }
  }

  std::list<std::string> locales;
  if (application->manifest_type() == Manifest::TYPE_WIDGET) {
    GetUserAgentLocales(GetSystemLocale(), locales);
    GetUserAgentLocales(application->GetManifest()->default_locale(), locales);
  }

  return new URLRequestApplicationJob(
      request,
      network_delegate,
      content::BrowserThread::GetBlockingPool()->
      GetTaskRunnerWithShutdownBehavior(
          base::SequencedWorkerPool::SKIP_ON_SHUTDOWN),
      application_id,
      directory_path,
      relative_path,
      content_security_policy,
      locales);
}

}  // namespace

linked_ptr<net::URLRequestJobFactory::ProtocolHandler>
CreateApplicationProtocolHandler(ApplicationService* service) {
  return linked_ptr<net::URLRequestJobFactory::ProtocolHandler>(
      new ApplicationProtocolHandler(service));
}

}  // namespace application
}  // namespace xwalk
