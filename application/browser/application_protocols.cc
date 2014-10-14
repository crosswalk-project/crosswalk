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

#if defined(OS_TIZEN)
#include <ss_manager.h>

#include "base/file_util.h"
#include "base/task_runner.h"
#include "net/base/file_stream.h"
#include "net/base/io_buffer.h"
#include "net/base/mime_util.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_job.h"
#include "net/url_request/url_request_status.h"

#include "xwalk/application/common/manifest_handlers/tizen_setting_handler.h"
#include "xwalk/application/common/tizen/encryption.h"
#endif

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
        content_security_policy_(content_security_policy),
        locales_(locales),
        resource_(application_id, directory_path, relative_path),
        relative_path_(relative_path),
        is_authority_match_(is_authority_match),
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

 protected:
  virtual ~URLRequestApplicationJob() {}

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
  bool is_authority_match_;
  base::WeakPtrFactory<URLRequestApplicationJob> weak_factory_;
};

#if defined(OS_TIZEN)
class URLRequestApplicationJobTizen : public URLRequestApplicationJob {
 public:
  URLRequestApplicationJobTizen(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate,
      const scoped_refptr<base::TaskRunner>& file_task_runner,
      const std::string& application_id,
      const base::FilePath& directory_path,
      const base::FilePath& relative_path,
      const std::string& content_security_policy,
      const std::list<std::string>& locales,
      bool is_authority_match,
      bool encrypted)
      : URLRequestApplicationJob(request, network_delegate, file_task_runner,
            application_id, directory_path, relative_path,
            content_security_policy, locales, is_authority_match),
        file_task_runner_(file_task_runner),
        stream_(new net::FileStream(file_task_runner)),
        encrypted_(encrypted),
        weak_ptr_factory_(this) {
  }

  void Start() override {
    if (!encrypted_)
      return URLRequestApplicationJob::Start();
    base::FilePath* read_file_path = new base::FilePath;
    resource_.SetLocales(locales_);
    bool posted = base::WorkerPool::PostTaskAndReply(
        FROM_HERE,
        base::Bind(&URLRequestApplicationJobTizen::ReadFilePath,
            weak_ptr_factory_.GetWeakPtr(), resource_,
            base::Unretained(read_file_path)),
        base::Bind(&URLRequestApplicationJobTizen::DidReadFilePath,
            weak_ptr_factory_.GetWeakPtr(), base::Owned(read_file_path)),
            true /* task is slow */);
    DCHECK(posted);
  }

  void Kill() override {
    if (!encrypted_)
      return URLRequestApplicationJob::Kill();
    weak_ptr_factory_.InvalidateWeakPtrs();
    URLRequestJob::Kill();
  }

  bool ReadRawData(net::IOBuffer* buf, int buf_size,
      int* bytes_read) override {
    if (!encrypted_)
      return URLRequestApplicationJob::ReadRawData(buf, buf_size, bytes_read);
    int remaining = plain_buffer_->size() - plain_buffer_offset_;
    if (buf_size > remaining)
      buf_size = remaining;
    if (!buf_size) {
      *bytes_read = 0;
      return true;
    }
    memcpy(buf->data(), plain_buffer_->data() + plain_buffer_offset_, buf_size);
    plain_buffer_offset_ += buf_size;
    *bytes_read = buf_size;
    return true;
  }

  bool GetMimeType(std::string* mime_type) const override {
    DCHECK(mime_type);
    if (!encrypted_)
      return URLRequestApplicationJob::GetMimeType(mime_type);
    if (!mime_type_.empty()) {
      *mime_type = mime_type_;
      return true;
    }
    return false;
  }

 private:
  void ReadFilePath(const ApplicationResource& resource,
      base::FilePath* file_path) {
    *file_path = resource.GetFilePath();
  }

  void DidReadFilePath(
      base::FilePath* read_file_path) {
    file_path_ = *read_file_path;
    if (file_path_.empty()) {
      NotifyHeadersComplete();
      return;
    }
    base::File::Info* file_info = new base::File::Info();
    file_task_runner_->PostTaskAndReply(FROM_HERE,
        base::Bind(&URLRequestApplicationJobTizen::FetchFileInfo,
            weak_ptr_factory_.GetWeakPtr(), file_path_,
            base::Unretained(file_info)),
        base::Bind(&URLRequestApplicationJobTizen::DidFetchFileInfo,
            weak_ptr_factory_.GetWeakPtr(), base::Owned(file_info)));
  }

  void FetchFileInfo(const base::FilePath& file_path,
      base::File::Info* file_info) {
    base::GetFileInfo(file_path, file_info);
  }

  void DidFetchFileInfo(const base::File::Info* file_info) {
    if (!file_info->size) {
      NotifyDone(net::URLRequestStatus(net::URLRequestStatus::FAILED,
          net::ERR_FILE_NOT_FOUND));
      return;
    }

    net::GetMimeTypeFromFile(file_path_, &mime_type_);
    cipher_buffer_ = new net::IOBufferWithSize(file_info->size);
    int flags = base::File::FLAG_OPEN |
                base::File::FLAG_READ |
                base::File::FLAG_ASYNC;
    int rv = stream_->Open(file_path_, flags,
        base::Bind(&URLRequestApplicationJobTizen::DidOpen,
            weak_ptr_factory_.GetWeakPtr()));
    if (rv != net::ERR_IO_PENDING)
      DidOpen(rv);
  }

  void DidOpen(int result) {
    if (result) {
      NotifyDone(net::URLRequestStatus(net::URLRequestStatus::FAILED, result));
      return;
    }
    int rv = stream_->Read(
        cipher_buffer_.get(),
        cipher_buffer_->size(),
        base::Bind(&URLRequestApplicationJobTizen::DidReadEncryptedData,
            weak_ptr_factory_.GetWeakPtr(), cipher_buffer_));
    if (rv != net::ERR_IO_PENDING)
      NotifyDone(net::URLRequestStatus(net::URLRequestStatus::FAILED, rv));
  }

  void DidReadEncryptedData(scoped_refptr<net::IOBufferWithSize> buf,
      int result) {
    stream_.reset();
    std::string plain_text;
    size_t read_len = 0;
    const char* filename = resource_.application_id().c_str();
    ssm_file_info_t sfi;
    ssm_getinfo(filename, &sfi, SSM_FLAG_DATA, filename);
    char* data = static_cast<char*>(
        malloc(sizeof(char) * (sfi.originSize + 1)));
    memset(data, 0x00, (sfi.originSize + 1));
    int ret = ssm_read(filename, data, sfi.originSize, &read_len,
        SSM_FLAG_SECRET_OPERATION, filename);
    std::string key_str(data, read_len);
    free(data);
    if (!ret && !DecryptData(buf->data(), buf->size(), key_str, &plain_text)) {
      NotifyDone(net::URLRequestStatus(net::URLRequestStatus::FAILED,
          net::ERR_REQUEST_RANGE_NOT_SATISFIABLE));
      return;
    }
    plain_buffer_ = new net::StringIOBuffer(plain_text);
    set_expected_content_size(plain_buffer_->size());
    plain_buffer_offset_ = 0;
    NotifyHeadersComplete();
  }

  const scoped_refptr<base::TaskRunner> file_task_runner_;
  scoped_ptr<net::FileStream> stream_;
  bool encrypted_;
  scoped_refptr<net::IOBufferWithSize> cipher_buffer_;
  scoped_refptr<net::StringIOBuffer> plain_buffer_;
  int plain_buffer_offset_;
  std::string mime_type_;
  base::WeakPtrFactory<URLRequestApplicationJobTizen> weak_ptr_factory_;
};
#endif

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
                         std::list<std::string>& ua_locales) {  // NOLINT
  if (sys_locale.empty())
    return;

  std::string locale = base::StringToLowerASCII(sys_locale);
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

  if (!application.get())
    return new net::URLRequestErrorJob(
        request, network_delegate, net::ERR_FILE_NOT_FOUND);

  base::FilePath relative_path =
      ApplicationURLToRelativeFilePath(request->url());
  base::FilePath directory_path;
  std::string content_security_policy;
  if (application.get()) {
    directory_path = application->path();

    const char* csp_key = GetCSPKey(application->manifest_type());
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

  std::list<std::string> locales;
  if (application.get() &&
      application->manifest_type() == Manifest::TYPE_WIDGET) {
    GetUserAgentLocales(GetSystemLocale(), locales);
    GetUserAgentLocales(application->GetManifest()->default_locale(), locales);
  }

#if defined(OS_TIZEN)
  TizenSettingInfo* info = static_cast<TizenSettingInfo*>(
      application->GetManifestData(application_widget_keys::kTizenSettingKey));
  bool encrypted = info &&
                   info->encryption_enabled() &&
                   RequiresEncryption(relative_path);
  return new URLRequestApplicationJobTizen(
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
      application.get(),
      encrypted);
#else
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
        application.get());
#endif
}

}  // namespace

linked_ptr<net::URLRequestJobFactory::ProtocolHandler>
CreateApplicationProtocolHandler(ApplicationService* service) {
  return linked_ptr<net::URLRequestJobFactory::ProtocolHandler>(
      new ApplicationProtocolHandler(service));
}

}  // namespace application
}  // namespace xwalk
