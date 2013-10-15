// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_protocols.h"

#include <algorithm>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
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
#include "xwalk/application/common/application.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/application_resource.h"
#include "xwalk/application/common/constants.h"

using content::ResourceRequestInfo;
using xwalk::application::Application;

namespace {

net::HttpResponseHeaders* BuildHttpHeaders(
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

class GeneratedMainDocumentJob: public net::URLRequestSimpleJob {
 public:
  GeneratedMainDocumentJob(net::URLRequest* request,
                           net::NetworkDelegate* network_delegate,
                           const base::FilePath& relative_path,
                           const scoped_refptr<const Application> application)
    : net::URLRequestSimpleJob(request, network_delegate),
      application_(application),
      mime_type_("text/html"),
      relative_path_(relative_path) {
  }

  // Overridden from URLRequestSimpleJob:
  virtual int GetData(std::string* mime_type,
                      std::string* charset,
                      std::string* data,
                      const net::CompletionCallback& callback) const OVERRIDE {
    *mime_type = mime_type_;
    *charset = "utf-8";
    *data = "<!DOCTYPE html>\n<body>\n";

    // TODO(xiang): use manifest handler instead of the raw data.
    const base::ListValue* main_scripts;
    application_->GetManifest()->GetList(
        xwalk::application_manifest_keys::kAppMainScriptsKey,
        &main_scripts);
    for (size_t i = 0; i < main_scripts->GetSize(); ++i) {
      std::string script;
      main_scripts->GetString(i, &script);
      *data += "<script src=\"";
      *data += script;
      *data += "\"></script>\n";
    }
    return net::OK;
  }

  virtual void GetResponseInfo(net::HttpResponseInfo* info) OVERRIDE {
    response_info_.headers = BuildHttpHeaders(mime_type_, "GET", relative_path_,
        relative_path_, true);
    *info = response_info_;
  }

 private:
  virtual ~GeneratedMainDocumentJob() {}

  scoped_refptr<const Application> application_;
  const std::string mime_type_;
  const base::FilePath relative_path_;
  net::HttpResponseInfo response_info_;
};

void ReadResourceFilePath(
    const xwalk::application::ApplicationResource& resource,
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
      bool is_authority_match)
      : net::URLRequestFileJob(
          request, network_delegate, base::FilePath(), file_task_runner),
      resource_(application_id, directory_path, relative_path),
      relative_path_(relative_path),
      is_authority_match_(is_authority_match),
      weak_factory_(this) {
  }

  virtual void GetResponseInfo(net::HttpResponseInfo* info) OVERRIDE {
    std::string mime_type;
    GetMimeType(&mime_type);
    std::string method = request()->method();
    response_info_.headers = BuildHttpHeaders(mime_type, method, file_path_,
        relative_path_, is_authority_match_);
    *info = response_info_;
  }

  virtual void Start() OVERRIDE {
    base::FilePath* read_file_path = new base::FilePath;

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
  bool is_authority_match_;
  xwalk::application::ApplicationResource resource_;
  base::WeakPtrFactory<URLRequestApplicationJob> weak_factory_;
};

class ApplicationProtocolHandler
    : public net::URLRequestJobFactory::ProtocolHandler {
 public:
  explicit ApplicationProtocolHandler(const Application* application)
    : application_(application) {
    CHECK(application_);
  }

  virtual ~ApplicationProtocolHandler() {}

  virtual net::URLRequestJob* MaybeCreateJob(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) const OVERRIDE;

 private:
  const Application* application_;
  DISALLOW_COPY_AND_ASSIGN(ApplicationProtocolHandler);
};

net::URLRequestJob*
ApplicationProtocolHandler::MaybeCreateJob(
    net::URLRequest* request, net::NetworkDelegate* network_delegate) const {
  std::string application_id = request->url().host();

  bool is_authority_match = application_id == application_->ID();
  base::FilePath relative_path =
      xwalk::application::ApplicationURLToRelativeFilePath(request->url());
  base::FilePath directory_path;
  if (is_authority_match)
    directory_path = application_->Path();

  std::string path = request->url().path();
  if (is_authority_match &&
      path.size() > 1 &&
      path.substr(1) == xwalk::application::kGeneratedMainDocumentFilename) {
    return new GeneratedMainDocumentJob(request, network_delegate,
        relative_path, application_);
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
      is_authority_match);
}

}  // namespace

linked_ptr<net::URLRequestJobFactory::ProtocolHandler>
CreateApplicationProtocolHandler(const Application* application) {
  return  linked_ptr<net::URLRequestJobFactory::ProtocolHandler>(
      new ApplicationProtocolHandler(application));
}
