// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_NET_ANDROID_STREAM_READER_URL_REQUEST_JOB_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_NET_ANDROID_STREAM_READER_URL_REQUEST_JOB_H_

#include <string>

#include "base/android/scoped_java_ref.h"
#include "base/files/file_path.h"
#include "base/location.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/thread_checker.h"
#include "net/http/http_byte_range.h"
#include "net/url_request/url_request_job.h"

namespace xwalk {
class InputStream;
class InputStreamReader;
}

namespace base {
class TaskRunner;
}

namespace net {
class HttpResponseInfo;
class URLRequest;
}

class InputStreamReaderWrapper;

// A request job that reads data from a Java InputStream.
class AndroidStreamReaderURLRequestJob : public net::URLRequestJob {
 public:
  /*
   * We use a delegate so that we can share code for this job in slightly
   * different contexts.
   */
  class Delegate {
   public:
    // This method is called from a worker thread, not from the IO thread.
    virtual scoped_ptr<xwalk::InputStream> OpenInputStream(
        JNIEnv* env,
        const GURL& url) = 0;

    // This method is called on the Job's thread if the result of calling
    // OpenInputStream was null.
    // Setting the |restart| parameter to true will cause the request to be
    // restarted with a new job.
    virtual void OnInputStreamOpenFailed(
        net::URLRequest* request,
        bool* restart) = 0;

    virtual bool GetMimeType(
        JNIEnv* env,
        net::URLRequest* request,
        xwalk::InputStream* stream,
        std::string* mime_type) = 0;

    virtual bool GetCharset(
        JNIEnv* env,
        net::URLRequest* request,
        xwalk::InputStream* stream,
        std::string* charset) = 0;

    virtual bool GetPackageName(
        JNIEnv* env,
        std::string* name) = 0;

    virtual ~Delegate() {}
  };

  AndroidStreamReaderURLRequestJob(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate,
      scoped_ptr<Delegate> delegate,
      const base::FilePath& relative_path,
      const std::string& content_security_policy);

  // URLRequestJob:
  virtual void Start() OVERRIDE;
  virtual void Kill() OVERRIDE;
  virtual bool ReadRawData(net::IOBuffer* buf,
                           int buf_size,
                           int* bytes_read) OVERRIDE;
  virtual void SetExtraRequestHeaders(
      const net::HttpRequestHeaders& headers) OVERRIDE;
  virtual bool GetMimeType(std::string* mime_type) const OVERRIDE;
  virtual bool GetCharset(std::string* charset) OVERRIDE;
  virtual int GetResponseCode() const OVERRIDE;
  virtual void GetResponseInfo(net::HttpResponseInfo* info) OVERRIDE;

 protected:
  virtual ~AndroidStreamReaderURLRequestJob();

  // Gets the TaskRunner for the worker thread.
  // Overridden in unittests.
  virtual base::TaskRunner* GetWorkerThreadRunner();

  // Creates an InputStreamReader instance.
  // Overridden in unittests to return a mock.
  virtual scoped_ptr<xwalk::InputStreamReader>
      CreateStreamReader(xwalk::InputStream* stream);

 private:
  void HeadersComplete(int status_code, const std::string& status_text);

  void OnInputStreamOpened(
      scoped_ptr<Delegate> delegate,
      scoped_ptr<xwalk::InputStream> input_stream);
  void OnReaderSeekCompleted(int content_size);
  void OnReaderReadCompleted(int bytes_read);

  net::HttpByteRange byte_range_;
  scoped_ptr<net::HttpResponseInfo> response_info_;
  scoped_ptr<Delegate> delegate_;
  const std::string mime_type_;
  base::FilePath relative_path_;
  std::string content_security_policy_;
  scoped_refptr<InputStreamReaderWrapper> input_stream_reader_wrapper_;
  base::WeakPtrFactory<AndroidStreamReaderURLRequestJob> weak_factory_;
  base::ThreadChecker thread_checker_;

  DISALLOW_COPY_AND_ASSIGN(AndroidStreamReaderURLRequestJob);
};

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_NET_ANDROID_STREAM_READER_URL_REQUEST_JOB_H_
