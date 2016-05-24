// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_web_resource_response_impl.h"

#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/memory/ptr_util.h"
#include "jni/XWalkWebResourceResponseInternal_jni.h"
#include "net/http/http_response_headers.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_job.h"
#include "xwalk/runtime/browser/android/net/input_stream_impl.h"

using base::android::ScopedJavaLocalRef;
using base::android::AppendJavaStringArrayToStringVector;

namespace xwalk {

XWalkWebResourceResponseImpl::XWalkWebResourceResponseImpl(
    const base::android::JavaRef<jobject>& obj)
  : java_object_(obj) {
}

XWalkWebResourceResponseImpl::~XWalkWebResourceResponseImpl() {
}

std::unique_ptr<InputStream>
XWalkWebResourceResponseImpl::GetInputStream(JNIEnv* env) const {
  ScopedJavaLocalRef<jobject> jstream =
      Java_XWalkWebResourceResponseInternal_getDataNative(
         env, java_object_.obj());
  if (jstream.is_null())
    return std::unique_ptr<InputStream>();
  return base::WrapUnique(new InputStreamImpl(jstream));
}

bool XWalkWebResourceResponseImpl::GetMimeType(
    JNIEnv* env, std::string* mime_type) const {
  ScopedJavaLocalRef<jstring> jstring_mime_type =
      Java_XWalkWebResourceResponseInternal_getMimeTypeNative(
         env, java_object_.obj());
  if (jstring_mime_type.is_null())
    return false;
  *mime_type = ConvertJavaStringToUTF8(jstring_mime_type);
  return true;
}

bool XWalkWebResourceResponseImpl::GetCharset(
    JNIEnv* env, std::string* charset) const {
  ScopedJavaLocalRef<jstring> jstring_charset =
      Java_XWalkWebResourceResponseInternal_getEncodingNative(
         env, java_object_.obj());
  if (jstring_charset.is_null())
    return false;
  *charset = ConvertJavaStringToUTF8(jstring_charset);
  return true;
}

bool XWalkWebResourceResponseImpl::GetPackageName(
    JNIEnv* env, std::string* name) const {
    // TODO(Xiaofeng): Implement this if we use intercepter for app scheme.
    return true;
}

bool XWalkWebResourceResponseImpl::GetStatusInfo(
    JNIEnv* env,
    int* status_code,
    std::string* reason_phrase) const {
  int status =
      Java_XWalkWebResourceResponseInternal_getStatusCodeNative(
         env, java_object_.obj());
  ScopedJavaLocalRef<jstring> jstring_reason_phrase =
      Java_XWalkWebResourceResponseInternal_getReasonPhraseNative(
         env, java_object_.obj());
  if (status < 100 || status >= 600 || jstring_reason_phrase.is_null())
    return false;
  *status_code = status;
  *reason_phrase = ConvertJavaStringToUTF8(jstring_reason_phrase);
  return true;
}

bool XWalkWebResourceResponseImpl::GetResponseHeaders(
    JNIEnv* env,
    net::HttpResponseHeaders* headers) const {
  ScopedJavaLocalRef<jobjectArray> jstringArray_headerNames =
      Java_XWalkWebResourceResponseInternal_getResponseHeaderNames(
         env, java_object_.obj());
  ScopedJavaLocalRef<jobjectArray> jstringArray_headerValues =
      Java_XWalkWebResourceResponseInternal_getResponseHeaderValues(
         env, java_object_.obj());
  if (jstringArray_headerNames.is_null() || jstringArray_headerValues.is_null())
    return false;
  std::vector<std::string> header_names;
  std::vector<std::string> header_values;
  AppendJavaStringArrayToStringVector(
      env, jstringArray_headerNames.obj(), &header_names);
  AppendJavaStringArrayToStringVector(
      env, jstringArray_headerValues.obj(), &header_values);
  DCHECK_EQ(header_values.size(), header_names.size());
  for (size_t i = 0; i < header_names.size(); ++i) {
    std::string header_line(header_names[i]);
    header_line.append(": ");
    header_line.append(header_values[i]);
    headers->AddHeader(header_line);
  }
  return true;
}

bool RegisterXWalkWebResourceResponse(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace xwalk
