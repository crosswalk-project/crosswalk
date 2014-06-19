// Copyright (c) 2012 The Chromium Authors. All rights reserved.
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
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/application_resource.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/manifest_handlers/csp_handler.h"

using content::BrowserThread;
using content::ResourceRequestInfo;

namespace xwalk {

namespace keys = application_manifest_keys;

namespace application {

namespace {

net::HttpResponseHeaders* BuildHttpHeaders(
    const std::string& content_security_policy,
    const std::string& mime_type, const std::string& method,
    const base::FilePath& file_path, const base::FilePath& relative_path,
    bool is_authority_match) {
  std::string raw_headers;
  if (method == "GET") {
    if (relative_path.empty())
      raw_headers.append("HTTP/1.1 400 Bad Request");
    else if (!is_authority_match)
      raw_headers.append("HTTP/1.1 403 Forbidden");
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
      const std::list<std::string>& locales,
      bool is_authority_match)
      : net::URLRequestFileJob(
          request, network_delegate, base::FilePath(), file_task_runner),
        relative_path_(relative_path),
        content_security_policy_(content_security_policy),
        is_authority_match_(is_authority_match),
        resource_(application_id, directory_path, relative_path),
        locales_(locales),
        weak_factory_(this) {
  }

  virtual void GetResponseInfo(net::HttpResponseInfo* info) OVERRIDE {
    std::string mime_type;
    GetMimeType(&mime_type);
    std::string method = request()->method();
    response_info_.headers = BuildHttpHeaders(
        content_security_policy_, mime_type, method, file_path_,
        relative_path_, is_authority_match_);
    *info = response_info_;
  }

  virtual void Start() OVERRIDE {
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

 private:
  virtual ~URLRequestApplicationJob() {}

  void OnFilePathRead(base::FilePath* read_file_path) {
    file_path_ = *read_file_path;
    if (file_path_.empty())
      NotifyHeadersComplete();
    else
      URLRequestFileJob::Start();
  }

  net::HttpResponseInfo response_info_;
  base::FilePath relative_path_;
  std::string content_security_policy_;
  bool is_authority_match_;
  ApplicationResource resource_;
  std::list<std::string> locales_;
  base::WeakPtrFactory<URLRequestApplicationJob> weak_factory_;
};

// This class is a thread-safe cache of active application's data.
// This class is used by ApplicationProtocolHandler as it lives on IO thread
// and hence cannot access ApplicationService directly.
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

  virtual void DidLaunchApplication(Application* app) OVERRIDE {
    base::AutoLock lock(lock_);
    cache_.insert(std::pair<std::string, scoped_refptr<ApplicationData> >(
        app->id(), app->data()));
  }

  virtual void WillDestroyApplication(Application* app) OVERRIDE {
    base::AutoLock lock(lock_);
    cache_.erase(app->id());
  }

 private:
  ApplicationData::ApplicationDataMap cache_;
  mutable base::Lock lock_;
};

class ApplicationProtocolHandler
    : public net::URLRequestJobFactory::ProtocolHandler {
 public:
  explicit ApplicationProtocolHandler(ApplicationService* service) {
    DCHECK(service);
    DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
    // ApplicationProtocolHandler lives longer than ApplicationService,
    // so we do not need to remove cache_ from ApplicationService
    // observers list.
    service->AddObserver(&cache_);
  }

  virtual ~ApplicationProtocolHandler() {}

  virtual net::URLRequestJob* MaybeCreateJob(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) const OVERRIDE;

 private:
  ApplicationDataCache cache_;
  DISALLOW_COPY_AND_ASSIGN(ApplicationProtocolHandler);
};

// The |locale| should be expanded to user agent locale.
// Such as, "en-us" will be expaned as "en-us, en".
void GetUserAgentLocales(const std::string& sys_locale,
                         std::list<std::string>& ua_locales) {
  if (sys_locale.empty())
    return;

  std::string locale = StringToLowerASCII(sys_locale);
  size_t position;
  do {
    ua_locales.push_back(locale);
    position = locale.find_last_of("-");
    locale = locale.substr(0, position);
  } while (position != std::string::npos);
}

net::URLRequestJob*
ApplicationProtocolHandler::MaybeCreateJob(
    net::URLRequest* request, net::NetworkDelegate* network_delegate) const {
  const std::string& application_id = request->url().host();
  scoped_refptr<ApplicationData> application =
      cache_.GetApplicationData(application_id);

  if (!application)
    return new net::URLRequestErrorJob(
        request, network_delegate, net::ERR_FILE_NOT_FOUND);

  base::FilePath relative_path =
      ApplicationURLToRelativeFilePath(request->url());
  base::FilePath directory_path;
  std::string content_security_policy;
  if (application) {
    directory_path = application->Path();

    const char* csp_key = GetCSPKey(application->GetPackageType());
    const CSPInfo* csp_info = static_cast<CSPInfo*>(
          application->GetManifestData(csp_key));
    if (csp_info) {
      const std::map<std::string, std::vector<std::string> >& policies =
          csp_info->GetDirectives();
      std::map<std::string, std::vector<std::string> >::const_iterator it =
          policies.begin();
      for (; it != policies.end(); ++it) {
        content_security_policy.append(
            it->first + ' ' + JoinString(it->second, ' ') + ';');
      }
    }
  }

  const std::string& path = request->url().path();

  std::list<std::string> locales;
  if (application && application->GetPackageType() == Package::WGT) {
    GetUserAgentLocales(
        xwalk::XWalkRunner::GetInstance()->GetLocale(), locales);
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
      locales,
      application);
}

}  // namespace

linked_ptr<net::URLRequestJobFactory::ProtocolHandler>
CreateApplicationProtocolHandler(ApplicationService* service) {
  return linked_ptr<net::URLRequestJobFactory::ProtocolHandler>(
      new ApplicationProtocolHandler(service));
}

}  // namespace application
}  // namespace xwalk
