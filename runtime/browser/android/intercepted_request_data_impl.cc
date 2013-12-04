// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/intercepted_request_data_impl.h"

#include <string>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "jni/InterceptedRequestData_jni.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_job.h"
#include "xwalk/runtime/browser/android/net/android_stream_reader_url_request_job.h"
#include "xwalk/runtime/browser/android/net/input_stream_impl.h"

using base::android::ScopedJavaLocalRef;

namespace xwalk {

namespace {

class StreamReaderJobDelegateImpl
    : public AndroidStreamReaderURLRequestJob::Delegate {
 public:
    StreamReaderJobDelegateImpl(
        const InterceptedRequestDataImpl* intercepted_request_data)
        : intercepted_request_data_impl_(intercepted_request_data) {
      DCHECK(intercepted_request_data_impl_);
    }

    virtual scoped_ptr<InputStream> OpenInputStream(
        JNIEnv* env,
        const GURL& url) OVERRIDE {
      return intercepted_request_data_impl_->GetInputStream(env).Pass();
    }

    virtual void OnInputStreamOpenFailed(net::URLRequest* request,
                                         bool* restart) OVERRIDE {
      *restart = false;
    }

    virtual bool GetMimeType(JNIEnv* env,
                             net::URLRequest* request,
                             xwalk::InputStream* stream,
                             std::string* mime_type) OVERRIDE {
      return intercepted_request_data_impl_->GetMimeType(env, mime_type);
    }

    virtual bool GetCharset(JNIEnv* env,
                            net::URLRequest* request,
                            xwalk::InputStream* stream,
                            std::string* charset) OVERRIDE {
      return intercepted_request_data_impl_->GetCharset(env, charset);
    }

 private:
    const InterceptedRequestDataImpl* intercepted_request_data_impl_;
};

}  // namespace

InterceptedRequestDataImpl::InterceptedRequestDataImpl(
    const base::android::JavaRef<jobject>& obj)
  : java_object_(obj) {
}

InterceptedRequestDataImpl::~InterceptedRequestDataImpl() {
}

scoped_ptr<InputStream>
InterceptedRequestDataImpl::GetInputStream(JNIEnv* env) const {
  ScopedJavaLocalRef<jobject> jstream =
      Java_InterceptedRequestData_getData(env, java_object_.obj());
  if (jstream.is_null())
    return scoped_ptr<InputStream>();
  return make_scoped_ptr<InputStream>(new InputStreamImpl(jstream));
}

bool InterceptedRequestDataImpl::GetMimeType(JNIEnv* env,
                                             std::string* mime_type) const {
  ScopedJavaLocalRef<jstring> jstring_mime_type =
      Java_InterceptedRequestData_getMimeType(env, java_object_.obj());
  if (jstring_mime_type.is_null())
    return false;
  *mime_type = ConvertJavaStringToUTF8(jstring_mime_type);
  return true;
}

bool InterceptedRequestDataImpl::GetCharset(
    JNIEnv* env, std::string* charset) const {
  ScopedJavaLocalRef<jstring> jstring_charset =
      Java_InterceptedRequestData_getCharset(env, java_object_.obj());
  if (jstring_charset.is_null())
    return false;
  *charset = ConvertJavaStringToUTF8(jstring_charset);
  return true;
}

bool RegisterInterceptedRequestData(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

net::URLRequestJob* InterceptedRequestDataImpl::CreateJobFor(
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate) const {
  scoped_ptr<AndroidStreamReaderURLRequestJob::Delegate>
      stream_reader_job_delegate_impl(new StreamReaderJobDelegateImpl(this));
  return new AndroidStreamReaderURLRequestJob(
      request, network_delegate, stream_reader_job_delegate_impl.Pass());
}

}  // namespace xwalk
