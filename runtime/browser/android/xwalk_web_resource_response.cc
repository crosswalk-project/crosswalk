// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_web_resource_response.h"

#include "base/strings/string_number_conversions.h"
#include "net/http/http_response_headers.h"
#include "xwalk/runtime/browser/android/net/android_stream_reader_url_request_job.h"
#include "xwalk/runtime/browser/android/net/input_stream.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

namespace xwalk {

namespace {

class StreamReaderJobDelegateImpl
    : public AndroidStreamReaderURLRequestJob::Delegate {
 public:
  StreamReaderJobDelegateImpl(
      scoped_ptr<XWalkWebResourceResponse> xwalk_web_resource_response)
      : xwalk_web_resource_response_(xwalk_web_resource_response.Pass()) {
    DCHECK(xwalk_web_resource_response_);
  }

  scoped_ptr<InputStream> OpenInputStream(JNIEnv* env,
                                          const GURL& url) override {
    return xwalk_web_resource_response_->GetInputStream(env).Pass();
  }

  void OnInputStreamOpenFailed(net::URLRequest* request,
                               bool* restart) override {
    *restart = false;
  }

  bool GetMimeType(JNIEnv* env,
                   net::URLRequest* request,
                   xwalk::InputStream* stream,
                   std::string* mime_type) override {
    return xwalk_web_resource_response_->GetMimeType(env, mime_type);
  }

  bool GetCharset(JNIEnv* env,
                  net::URLRequest* request,
                  xwalk::InputStream* stream,
                  std::string* charset) override {
    return xwalk_web_resource_response_->GetCharset(env, charset);
  }

  bool GetPackageName(JNIEnv* env,
                      std::string* name) override {
    return xwalk_web_resource_response_->GetPackageName(env, name);
  }

  void AppendResponseHeaders(JNIEnv* env,
                             net::HttpResponseHeaders* headers) override {
    int status_code;
    std::string reason_phrase;
    if (xwalk_web_resource_response_->GetStatusInfo(
            env, &status_code, &reason_phrase)) {
      std::string status_line("HTTP/1.1 ");
      status_line.append(base::IntToString(status_code));
      status_line.append(" ");
      status_line.append(reason_phrase);
      headers->ReplaceStatusLine(status_line);
    }
    xwalk_web_resource_response_->GetResponseHeaders(env, headers);
  }

 private:
  scoped_ptr<XWalkWebResourceResponse> xwalk_web_resource_response_;
};

}  // namespace

// static
net::URLRequestJob* XWalkWebResourceResponse::CreateJobFor(
    scoped_ptr<XWalkWebResourceResponse> xwalk_web_resource_response,
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate) {
  DCHECK(xwalk_web_resource_response);
  DCHECK(request);
  DCHECK(network_delegate);

  XWalkBrowserContext* browser_context =
      XWalkRunner::GetInstance()->browser_context();
  std::string content_security_policy = browser_context->GetCSPString();

  return new AndroidStreamReaderURLRequestJob(
      request,
      network_delegate,
      make_scoped_ptr(
          new StreamReaderJobDelegateImpl(xwalk_web_resource_response.Pass())),
          content_security_policy);
}

}  // namespace xwalk
