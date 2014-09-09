// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_encrypted_file_job_tizen.h"

#include "base/strings/string_util.h"
#include "base/task_runner.h"
#include "base/threading/worker_pool.h"
#include "base/threading/sequenced_worker_pool.h"
#include "net/base/file_stream.h"
#include "net/base/mime_util.h"
#include "net/base/io_buffer.h"
#include "net/base/net_errors.h"
#include "net/http/http_response_headers.h"
#include "net/http/http_response_info.h"
#include "net/url_request/url_request_status.h"
#include "xwalk/application/common/encryption_tizen.h"

namespace xwalk {
namespace application {

URLRequestAppEncryptedFileJob::URLRequestAppEncryptedFileJob(
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate,
    const scoped_refptr<base::TaskRunner>& file_task_runner,
    const std::string& application_id,
    const base::FilePath& directory_path,
    const base::FilePath& relative_path,
    const std::string& content_security_policy,
    const std::list<std::string>& locales)
    : net::URLRequestJob(request, network_delegate),
      file_task_runner_(file_task_runner),
      stream_(new net::FileStream(file_task_runner)),
      relative_path_(relative_path),
      resource_(application_id, directory_path, relative_path),
      content_security_policy_(content_security_policy),
      locales_(locales),
      weak_ptr_factory_(this) {
}

URLRequestAppEncryptedFileJob::~URLRequestAppEncryptedFileJob() {}

void URLRequestAppEncryptedFileJob::GetResponseInfo(
    net::HttpResponseInfo* info) {
  std::string headers("HTTP/1.1 200 OK");
  if (!content_security_policy_.empty()) {
    headers.append(1, '\0');
    headers.append("Content-Security-Policy: ");
    headers.append(content_security_policy_);
  }
  headers.append(1, '\0');
  headers.append("Access-Control-Allow-Origin: *");
  std::string mime_type;
  GetMimeType(&mime_type);
  if (!mime_type.empty()) {
    headers.append(1, '\0');
    headers.append("Content-Type: ");
    headers.append(mime_type);
  }
  headers.append(2, '\0');
  info->headers = new net::HttpResponseHeaders(headers);
}

void URLRequestAppEncryptedFileJob::Start() {
  base::FilePath* read_file_path = new base::FilePath;
  resource_.SetLocales(locales_);
  bool posted = base::WorkerPool::PostTaskAndReply(
      FROM_HERE,
      base::Bind(&URLRequestAppEncryptedFileJob::ReadFilePath,
                 weak_ptr_factory_.GetWeakPtr(),
                 resource_,
                 base::Unretained(read_file_path)),
      base::Bind(&URLRequestAppEncryptedFileJob::DidReadFilePath,
                 weak_ptr_factory_.GetWeakPtr(),
                 base::Owned(read_file_path)),
      true /* task is slow */);
  DCHECK(posted);
}

void URLRequestAppEncryptedFileJob::Kill() {
  weak_ptr_factory_.InvalidateWeakPtrs();
  URLRequestJob::Kill();
}

bool URLRequestAppEncryptedFileJob::ReadRawData(net::IOBuffer* buf,
                                                int buf_size,
                                                int* bytes_read) {
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

bool URLRequestAppEncryptedFileJob::IsRedirectResponse(GURL* location,
                                                       int* http_status_code) {
  return false;
}

bool URLRequestAppEncryptedFileJob::GetMimeType(std::string* mime_type) const {
  DCHECK(mime_type);
  if (!mime_type_.empty()) {
    *mime_type = mime_type_;
    return true;
  }
  return false;
}

void URLRequestAppEncryptedFileJob::ReadFilePath(
    const ApplicationResource& resource,
    base::FilePath* file_path) {
  *file_path = resource.GetFilePath();
}

void URLRequestAppEncryptedFileJob::DidReadFilePath(
    base::FilePath* read_file_path) {
  file_path_ = *read_file_path;
  if (file_path_.empty()) {
    NotifyHeadersComplete();
    return;
  }
  base::File::Info* file_info = new base::File::Info();
  file_task_runner_->PostTaskAndReply(
      FROM_HERE,
      base::Bind(&URLRequestAppEncryptedFileJob::FetchFileInfo,
                 weak_ptr_factory_.GetWeakPtr(),
                 file_path_,
                 base::Unretained(file_info)),
      base::Bind(&URLRequestAppEncryptedFileJob::DidFetchFileInfo,
                 weak_ptr_factory_.GetWeakPtr(),
                 base::Owned(file_info)));
}

void URLRequestAppEncryptedFileJob::FetchFileInfo(
    const base::FilePath& file_path,
    base::File::Info* file_info) {
  base::GetFileInfo(file_path, file_info);
}

void URLRequestAppEncryptedFileJob::DidFetchFileInfo(
    const base::File::Info* file_info) {
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
  int rv = stream_->Open(file_path_,
                         flags,
                         base::Bind(&URLRequestAppEncryptedFileJob::DidOpen,
                                    weak_ptr_factory_.GetWeakPtr()));
  if (rv != net::ERR_IO_PENDING)
    DidOpen(rv);
}

void URLRequestAppEncryptedFileJob::DidOpen(int result) {
  if (result) {
    NotifyDone(net::URLRequestStatus(net::URLRequestStatus::FAILED, result));
    return;
  }
  int rv = stream_->Read(
      cipher_buffer_.get(),
      cipher_buffer_->size(),
      base::Bind(&URLRequestAppEncryptedFileJob::DidReadEncryptedData,
                 weak_ptr_factory_.GetWeakPtr(),
                 cipher_buffer_));
  if (rv != net::ERR_IO_PENDING)
    NotifyDone(net::URLRequestStatus(net::URLRequestStatus::FAILED, rv));
}

void URLRequestAppEncryptedFileJob::DidReadEncryptedData(
    scoped_refptr<net::IOBufferWithSize> buf,
    int result) {
  stream_.reset();
  std::string plain_text;
  if (!DecryptData(buf->data(), buf->size(), &plain_text)) {
    NotifyDone(net::URLRequestStatus(net::URLRequestStatus::FAILED,
        net::ERR_REQUEST_RANGE_NOT_SATISFIABLE));
    return;
  }
  plain_buffer_ = new net::StringIOBuffer(plain_text);
  set_expected_content_size(plain_buffer_->size());
  plain_buffer_offset_ = 0;
  NotifyHeadersComplete();
}

}  // namespace application
}  // namespace xwalk
