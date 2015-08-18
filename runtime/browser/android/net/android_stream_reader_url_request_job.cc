// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/net/android_stream_reader_url_request_job.h"

#include <string>
#include <vector>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/task_runner.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/thread.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/io_buffer.h"
#include "net/base/mime_util.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "net/http/http_response_headers.h"
#include "net/http/http_response_info.h"
#include "net/http/http_util.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_job_manager.h"
#include "xwalk/runtime/browser/android/net/input_stream.h"
#include "xwalk/runtime/browser/android/net/input_stream_reader.h"
#include "xwalk/runtime/browser/android/net/url_constants.h"

using base::android::AttachCurrentThread;
using base::android::DetachFromVM;
using base::PostTaskAndReplyWithResult;
using content::BrowserThread;
using xwalk::InputStream;
using xwalk::InputStreamReader;

namespace {

const int kHTTPOk = 200;
const int kHTTPBadRequest = 400;
const int kHTTPForbidden = 403;
const int kHTTPNotFound = 404;
const int kHTTPNotImplemented = 501;

const char kHTTPOkText[] = "OK";
const char kHTTPBadRequestText[] = "Bad Request";
const char kHTTPForbiddenText[] = "Forbidden";
const char kHTTPNotFoundText[] = "Not Found";
const char kHTTPNotImplementedText[] = "Not Implemented";

}  // namespace

// The requests posted to the worker thread might outlive the job. Thread-safe
// ref counting is used to ensure that the InputStream and InputStreamReader
// members of this class are still there when the closure is run on the worker
// thread.
class InputStreamReaderWrapper
    : public base::RefCountedThreadSafe<InputStreamReaderWrapper> {
 public:
  InputStreamReaderWrapper(
      scoped_ptr<InputStream> input_stream,
      scoped_ptr<InputStreamReader> input_stream_reader)
      : input_stream_(input_stream.Pass()),
        input_stream_reader_(input_stream_reader.Pass()) {
    DCHECK(input_stream_);
    DCHECK(input_stream_reader_);
  }

  xwalk::InputStream* input_stream() {
    return input_stream_.get();
  }

  int Seek(const net::HttpByteRange& byte_range) {
    return input_stream_reader_->Seek(byte_range);
  }

  int ReadRawData(net::IOBuffer* buffer, int buffer_size) {
    return input_stream_reader_->ReadRawData(buffer, buffer_size);
  }

 private:
  friend class base::RefCountedThreadSafe<InputStreamReaderWrapper>;
  ~InputStreamReaderWrapper() {}

  scoped_ptr<xwalk::InputStream> input_stream_;
  scoped_ptr<xwalk::InputStreamReader> input_stream_reader_;

  DISALLOW_COPY_AND_ASSIGN(InputStreamReaderWrapper);
};

AndroidStreamReaderURLRequestJob::AndroidStreamReaderURLRequestJob(
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate,
    scoped_ptr<Delegate> delegate,
    const std::string& content_security_policy)
    : URLRequestJob(request, network_delegate),
      delegate_(delegate.Pass()),
      content_security_policy_(content_security_policy),
      weak_factory_(this) {
  DCHECK(delegate_);
}

AndroidStreamReaderURLRequestJob::~AndroidStreamReaderURLRequestJob() {
}

namespace {

typedef base::Callback<
    void(scoped_ptr<AndroidStreamReaderURLRequestJob::Delegate>,
         scoped_ptr<InputStream>)> OnInputStreamOpenedCallback;

// static
void OpenInputStreamOnWorkerThread(
    scoped_refptr<base::SingleThreadTaskRunner> job_thread_runner,
    scoped_ptr<AndroidStreamReaderURLRequestJob::Delegate> delegate,
    const GURL& url,
    OnInputStreamOpenedCallback callback) {

  JNIEnv* env = AttachCurrentThread();
  DCHECK(env);

  scoped_ptr<InputStream> input_stream = delegate->OpenInputStream(env, url);
  job_thread_runner->PostTask(FROM_HERE,
                              base::Bind(callback,
                                         base::Passed(delegate.Pass()),
                                         base::Passed(input_stream.Pass())));

  DetachFromVM();
}

}  // namespace

void AndroidStreamReaderURLRequestJob::Start() {
  DCHECK(thread_checker_.CalledOnValidThread());

  GURL url = request()->url();
  // Generate the HTTP header response for app scheme.
  if (url.SchemeIs(xwalk::kAppScheme)) {
    // Once the unpermitted request was received, the job should be marked
    // as complete, and the function should return, so that the request task
    // won't be posted.
    if (request()->method() != "GET") {
      HeadersComplete(kHTTPNotImplemented, kHTTPNotImplementedText);
      return;
    } else if (url.path().empty()) {
      HeadersComplete(kHTTPBadRequest, kHTTPBadRequestText);
      return;
    } else {
      JNIEnv* env = AttachCurrentThread();
      DCHECK(env);
      std::string package_name;
      delegate_->GetPackageName(env, &package_name);

      // The host should be the same as the lower case of the package
      // name, otherwise the resource request should be rejected.
      // TODO(Xingnan): More permission control policy will be added here,
      // if it's needed.
      if (request()->url().host() != base::StringToLowerASCII(package_name)) {
        HeadersComplete(kHTTPForbidden, kHTTPForbiddenText);
        return;
      }
    }
  }

  // Start reading asynchronously so that all error reporting and data
  // callbacks happen as they would for network requests.
  SetStatus(net::URLRequestStatus(net::URLRequestStatus::IO_PENDING,
                                  net::ERR_IO_PENDING));

  // This could be done in the InputStreamReader but would force more
  // complex synchronization in the delegate.
  GetWorkerThreadRunner()->PostTask(
      FROM_HERE,
      base::Bind(
          &OpenInputStreamOnWorkerThread,
          base::MessageLoop::current()->task_runner(),
          // This is intentional - the job could be deleted while the callback
          // is executing on the background thread.
          // The delegate will be "returned" to the job once the InputStream
          // open attempt is completed.
          base::Passed(&delegate_),
          request()->url(),
          base::Bind(&AndroidStreamReaderURLRequestJob::OnInputStreamOpened,
                     weak_factory_.GetWeakPtr())));
}

void AndroidStreamReaderURLRequestJob::Kill() {
  DCHECK(thread_checker_.CalledOnValidThread());
  weak_factory_.InvalidateWeakPtrs();
  URLRequestJob::Kill();
}

scoped_ptr<InputStreamReader>
AndroidStreamReaderURLRequestJob::CreateStreamReader(InputStream* stream) {
  return make_scoped_ptr(new InputStreamReader(stream));
}

void AndroidStreamReaderURLRequestJob::OnInputStreamOpened(
      scoped_ptr<Delegate> returned_delegate,
      scoped_ptr<xwalk::InputStream> input_stream) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(returned_delegate);
  delegate_ = returned_delegate.Pass();

  if (!input_stream) {
    bool restart_required = false;
    delegate_->OnInputStreamOpenFailed(request(), &restart_required);
    if (restart_required) {
      NotifyRestartRequired();
    } else {
      // Clear the IO_PENDING status set in Start().
      SetStatus(net::URLRequestStatus());
      HeadersComplete(kHTTPNotFound, kHTTPNotFoundText);
    }
    return;
  }

  scoped_ptr<InputStreamReader> input_stream_reader(
      CreateStreamReader(input_stream.get()));
  DCHECK(input_stream_reader);

  DCHECK(!input_stream_reader_wrapper_.get());
  input_stream_reader_wrapper_ = new InputStreamReaderWrapper(
      input_stream.Pass(), input_stream_reader.Pass());

  PostTaskAndReplyWithResult(
      GetWorkerThreadRunner(),
      FROM_HERE,
      base::Bind(&InputStreamReaderWrapper::Seek,
                 input_stream_reader_wrapper_,
                 byte_range_),
      base::Bind(&AndroidStreamReaderURLRequestJob::OnReaderSeekCompleted,
                 weak_factory_.GetWeakPtr()));
}

void AndroidStreamReaderURLRequestJob::OnReaderSeekCompleted(int result) {
  DCHECK(thread_checker_.CalledOnValidThread());
  // Clear the IO_PENDING status set in Start().
  SetStatus(net::URLRequestStatus());
  if (result >= 0) {
    set_expected_content_size(result);
    HeadersComplete(kHTTPOk, kHTTPOkText);
  } else {
    NotifyDone(net::URLRequestStatus(net::URLRequestStatus::FAILED, result));
  }
}

void AndroidStreamReaderURLRequestJob::OnReaderReadCompleted(int result) {
  DCHECK(thread_checker_.CalledOnValidThread());
  // The URLRequest API contract requires that:
  // * NotifyDone be called once, to set the status code, indicate the job is
  //   finished (there will be no further IO),
  // * NotifyReadComplete be called if false is returned from ReadRawData to
  //   indicate that the IOBuffer will not be used by the job anymore.
  // There might be multiple calls to ReadRawData (and thus multiple calls to
  // NotifyReadComplete), which is why NotifyDone is called only on errors
  // (result < 0) and end of data (result == 0).
  if (result == 0) {
    NotifyDone(net::URLRequestStatus());
  } else if (result < 0) {
    NotifyDone(net::URLRequestStatus(net::URLRequestStatus::FAILED, result));
  } else {
    // Clear the IO_PENDING status.
    SetStatus(net::URLRequestStatus());
  }
  NotifyReadComplete(result);
}

base::TaskRunner* AndroidStreamReaderURLRequestJob::GetWorkerThreadRunner() {
  return static_cast<base::TaskRunner*>(BrowserThread::GetBlockingPool());
}

bool AndroidStreamReaderURLRequestJob::ReadRawData(net::IOBuffer* dest,
                                                   int dest_size,
                                                   int* bytes_read) {
  DCHECK(thread_checker_.CalledOnValidThread());
  if (!input_stream_reader_wrapper_.get()) {
    // This will happen if opening the InputStream fails in which case the
    // error is communicated by setting the HTTP response status header rather
    // than failing the request during the header fetch phase.
    *bytes_read = 0;
    return true;
  }

  PostTaskAndReplyWithResult(
      GetWorkerThreadRunner(),
      FROM_HERE,
      base::Bind(&InputStreamReaderWrapper::ReadRawData,
                 input_stream_reader_wrapper_,
                 make_scoped_refptr(dest),
                 dest_size),
      base::Bind(&AndroidStreamReaderURLRequestJob::OnReaderReadCompleted,
                 weak_factory_.GetWeakPtr()));

  SetStatus(net::URLRequestStatus(net::URLRequestStatus::IO_PENDING,
                                  net::ERR_IO_PENDING));
  return false;
}

bool AndroidStreamReaderURLRequestJob::GetMimeType(
    std::string* mime_type) const {
  DCHECK(thread_checker_.CalledOnValidThread());
  JNIEnv* env = AttachCurrentThread();
  DCHECK(env);

  if (!input_stream_reader_wrapper_.get())
    return false;

  // Since it's possible for this call to alter the InputStream a
  // Seek or ReadRawData operation running in the background is not permitted.
  DCHECK(!request_->status().is_io_pending());

  return delegate_->GetMimeType(
      env, request(), input_stream_reader_wrapper_->input_stream(), mime_type);
}

bool AndroidStreamReaderURLRequestJob::GetCharset(std::string* charset) {
  DCHECK(thread_checker_.CalledOnValidThread());
  JNIEnv* env = AttachCurrentThread();
  DCHECK(env);

  if (!input_stream_reader_wrapper_.get())
    return false;

  // Since it's possible for this call to alter the InputStream a
  // Seek or ReadRawData operation running in the background is not permitted.
  DCHECK(!request_->status().is_io_pending());

  return delegate_->GetCharset(
      env, request(), input_stream_reader_wrapper_->input_stream(), charset);
}

void AndroidStreamReaderURLRequestJob::HeadersComplete(
    int status_code,
    const std::string& status_text) {
  std::string status("HTTP/1.1 ");
  status.append(base::IntToString(status_code));
  status.append(" ");
  status.append(status_text);
  // HttpResponseHeaders expects its input string to be terminated by two NULs.
  status.append("\0\0", 2);
  net::HttpResponseHeaders* headers = new net::HttpResponseHeaders(status);

  if (status_code == kHTTPOk) {
    if (expected_content_size() != -1) {
      std::string content_length_header(
          net::HttpRequestHeaders::kContentLength);
      content_length_header.append(": ");
      content_length_header.append(
          base::Int64ToString(expected_content_size()));
      headers->AddHeader(content_length_header);
    }

    std::string mime_type;
    if (GetMimeType(&mime_type) && !mime_type.empty()) {
      std::string content_type_header(net::HttpRequestHeaders::kContentType);
      content_type_header.append(": ");
      content_type_header.append(mime_type);
      headers->AddHeader(content_type_header);
    }

    if (!content_security_policy_.empty()) {
      std::string content_security_policy("Content-Security-Policy: ");
      content_security_policy.append(content_security_policy_);
      headers->AddHeader(content_security_policy);
    }
  }

  response_info_.reset(new net::HttpResponseInfo());
  response_info_->headers = headers;

  NotifyHeadersComplete();
}

int AndroidStreamReaderURLRequestJob::GetResponseCode() const {
  if (response_info_)
    return response_info_->headers->response_code();
  return URLRequestJob::GetResponseCode();
}

void AndroidStreamReaderURLRequestJob::GetResponseInfo(
    net::HttpResponseInfo* info) {
  if (response_info_)
    *info = *response_info_;
}

void AndroidStreamReaderURLRequestJob::SetExtraRequestHeaders(
    const net::HttpRequestHeaders& headers) {
  std::string range_header;
  if (headers.GetHeader(net::HttpRequestHeaders::kRange, &range_header)) {
    // We only extract the "Range" header so that we know how many bytes in the
    // stream to skip and how many to read after that.
    std::vector<net::HttpByteRange> ranges;
    if (net::HttpUtil::ParseRangeHeader(range_header, &ranges)) {
      if (ranges.size() == 1) {
        byte_range_ = ranges[0];
      } else {
        // We don't support multiple range requests in one single URL request,
        // because we need to do multipart encoding here.
        NotifyDone(net::URLRequestStatus(
            net::URLRequestStatus::FAILED,
            net::ERR_REQUEST_RANGE_NOT_SATISFIABLE));
      }
    }
  }
}
