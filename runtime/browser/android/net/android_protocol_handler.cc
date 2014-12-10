// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/net/android_protocol_handler.h"

#include <string>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/jni_weak_ref.h"
#include "base/strings/string_util.h"
#include "content/public/common/url_constants.h"
#include "jni/AndroidProtocolHandler_jni.h"
#include "net/base/io_buffer.h"
#include "net/base/mime_util.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "net/http/http_util.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_intercepting_job_factory.h"
#include "net/url_request/url_request_interceptor.h"
#include "url/gurl.h"
#include "xwalk/runtime/browser/android/net/android_stream_reader_url_request_job.h"
#include "xwalk/runtime/browser/android/net/input_stream_impl.h"
#include "xwalk/runtime/browser/android/net/url_constants.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

using base::android::AttachCurrentThread;
using base::android::ClearException;
using base::android::ConvertUTF8ToJavaString;
using base::android::ScopedJavaGlobalRef;
using base::android::ScopedJavaLocalRef;
using xwalk::InputStream;
using xwalk::InputStreamImpl;

namespace {

// Override resource context for reading resource and asset files. Used for
// testing.
JavaObjectWeakGlobalRef* g_resource_context = NULL;

void ResetResourceContext(JavaObjectWeakGlobalRef* ref) {
  if (g_resource_context)
    delete g_resource_context;

  g_resource_context = ref;
}

void* kPreviouslyFailedKey = &kPreviouslyFailedKey;

void MarkRequestAsFailed(net::URLRequest* request) {
  request->SetUserData(kPreviouslyFailedKey,
                       new base::SupportsUserData::Data());
}

bool HasRequestPreviouslyFailed(net::URLRequest* request) {
  return request->GetUserData(kPreviouslyFailedKey) != NULL;
}

class AndroidStreamReaderURLRequestJobDelegateImpl
    : public AndroidStreamReaderURLRequestJob::Delegate {
 public:
  AndroidStreamReaderURLRequestJobDelegateImpl();

  scoped_ptr<InputStream> OpenInputStream(
      JNIEnv* env,
      const GURL& url) override;

  void OnInputStreamOpenFailed(net::URLRequest* request,
                               bool* restart) override;

  bool GetMimeType(JNIEnv* env,
                   net::URLRequest* request,
                   InputStream* stream,
                   std::string* mime_type) override;

  bool GetCharset(JNIEnv* env,
                  net::URLRequest* request,
                  InputStream* stream,
                  std::string* charset) override;

  bool GetPackageName(JNIEnv* env,
                      std::string* name) override;

  virtual ~AndroidStreamReaderURLRequestJobDelegateImpl();
};

class AndroidRequestInterceptorBase : public net::URLRequestInterceptor {
 public:
  net::URLRequestJob* MaybeInterceptRequest(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) const override;

  virtual bool ShouldHandleRequest(const net::URLRequest* request) const = 0;
};

class AssetFileRequestInterceptor : public AndroidRequestInterceptorBase {
 public:
  AssetFileRequestInterceptor();

  virtual ~AssetFileRequestInterceptor();
  bool ShouldHandleRequest(
      const net::URLRequest* request) const override;

 private:
  // file:///android_asset/
  const std::string asset_prefix_;
  // file:///android_res/
  const std::string resource_prefix_;
};

// Protocol handler for app:// scheme requests.
class AppSchemeRequestInterceptor : public AndroidRequestInterceptorBase {
 public:
  AppSchemeRequestInterceptor();
  bool ShouldHandleRequest(
      const net::URLRequest* request) const override;
};

// Protocol handler for content:// scheme requests.
class ContentSchemeRequestInterceptor : public AndroidRequestInterceptorBase {
 public:
  ContentSchemeRequestInterceptor();
  bool ShouldHandleRequest(
      const net::URLRequest* request) const override;
};

static ScopedJavaLocalRef<jobject> GetResourceContext(JNIEnv* env) {
  if (g_resource_context)
    return g_resource_context->get(env);
  ScopedJavaLocalRef<jobject> context;
  // We have to reset as GetApplicationContext() returns a jobject with a
  // global ref. The constructor that takes a jobject would expect a local ref
  // and would assert.
  context.Reset(env, base::android::GetApplicationContext());
  return context;
}

// AndroidStreamReaderURLRequestJobDelegateImpl -------------------------------

AndroidStreamReaderURLRequestJobDelegateImpl::
    AndroidStreamReaderURLRequestJobDelegateImpl() {}

AndroidStreamReaderURLRequestJobDelegateImpl::
~AndroidStreamReaderURLRequestJobDelegateImpl() {
}

scoped_ptr<InputStream>
AndroidStreamReaderURLRequestJobDelegateImpl::OpenInputStream(
    JNIEnv* env, const GURL& url) {
  DCHECK(url.is_valid());
  DCHECK(env);

  // Open the input stream.
  ScopedJavaLocalRef<jstring> jurl =
      ConvertUTF8ToJavaString(env, url.spec());
  ScopedJavaLocalRef<jobject> stream =
      xwalk::Java_AndroidProtocolHandler_open(
          env,
          GetResourceContext(env).obj(),
          jurl.obj());

  // Check and clear pending exceptions.
  if (ClearException(env) || stream.is_null()) {
    DLOG(ERROR) << "Unable to open input stream for Android URL";
    return scoped_ptr<InputStream>();
  }
  return make_scoped_ptr<InputStream>(new InputStreamImpl(stream));
}

void AndroidStreamReaderURLRequestJobDelegateImpl::OnInputStreamOpenFailed(
    net::URLRequest* request,
    bool* restart) {
  DCHECK(!HasRequestPreviouslyFailed(request));
  MarkRequestAsFailed(request);
  *restart = true;
}

bool AndroidStreamReaderURLRequestJobDelegateImpl::GetMimeType(
    JNIEnv* env,
    net::URLRequest* request,
    xwalk::InputStream* stream,
    std::string* mime_type) {
  DCHECK(env);
  DCHECK(request);
  DCHECK(mime_type);

  // Query the mime type from the Java side. It is possible for the query to
  // fail, as the mime type cannot be determined for all supported schemes.
  ScopedJavaLocalRef<jstring> url =
      ConvertUTF8ToJavaString(env, request->url().spec());
  const InputStreamImpl* stream_impl =
      InputStreamImpl::FromInputStream(stream);
  ScopedJavaLocalRef<jstring> returned_type =
      xwalk::Java_AndroidProtocolHandler_getMimeType(
          env,
          GetResourceContext(env).obj(),
          stream_impl->jobj(), url.obj());
  if (ClearException(env) || returned_type.is_null())
    return false;

  *mime_type = base::android::ConvertJavaStringToUTF8(returned_type);
  return true;
}

bool AndroidStreamReaderURLRequestJobDelegateImpl::GetCharset(
    JNIEnv* env,
    net::URLRequest* request,
    xwalk::InputStream* stream,
    std::string* charset) {
  // TODO(shouqun): We should probably be getting this from the managed side.
  return false;
}

bool AndroidStreamReaderURLRequestJobDelegateImpl::GetPackageName(
    JNIEnv* env,
    std::string* name) {
  ScopedJavaLocalRef<jstring> returned_name =
      xwalk::Java_AndroidProtocolHandler_getPackageName(
          env,
          GetResourceContext(env).obj());

  if (ClearException(env) || returned_name.is_null())
    return false;

  *name = base::android::ConvertJavaStringToUTF8(returned_name);
  return true;
}

// AndroidRequestInterceptorBase ----------------------------------------------

net::URLRequestJob* AndroidRequestInterceptorBase::MaybeInterceptRequest(
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate) const {
  if (!ShouldHandleRequest(request))
    return NULL;

  // For WebViewClassic compatibility this job can only accept URLs that can be
  // opened. URLs that cannot be opened should be resolved by the next handler.
  //
  // If a request is initially handled here but the job fails due to it being
  // unable to open the InputStream for that request the request is marked as
  // previously failed and restarted.
  // Restarting a request involves creating a new job for that request. This
  // handler will ignore requests know to have previously failed to 1) prevent
  // an infinite loop, 2) ensure that the next handler in line gets the
  // opportunity to create a job for the request.
  if (HasRequestPreviouslyFailed(request))
    return NULL;

  scoped_ptr<AndroidStreamReaderURLRequestJobDelegateImpl> reader_delegate(
      new AndroidStreamReaderURLRequestJobDelegateImpl());

  xwalk::XWalkBrowserContext* browser_context =
      xwalk::XWalkRunner::GetInstance()->browser_context();
  std::string content_security_policy = browser_context->GetCSPString();

  return new AndroidStreamReaderURLRequestJob(
      request,
      network_delegate,
      reader_delegate.Pass(),
      content_security_policy);
}

// AssetFileRequestInterceptor ----------------------------------------------

AssetFileRequestInterceptor::AssetFileRequestInterceptor()
    : asset_prefix_(std::string(url::kFileScheme) +
                    std::string(url::kStandardSchemeSeparator) +
                    xwalk::kAndroidAssetPath),
      resource_prefix_(std::string(url::kFileScheme) +
                       std::string(url::kStandardSchemeSeparator) +
                       xwalk::kAndroidResourcePath) {
}

AssetFileRequestInterceptor::~AssetFileRequestInterceptor() {
}

bool AssetFileRequestInterceptor::ShouldHandleRequest(
    const net::URLRequest* request) const {
  if (!request->url().SchemeIsFile())
    return false;

  const std::string& url = request->url().spec();
  if (!StartsWithASCII(url, asset_prefix_, /*case_sensitive=*/ true) &&
      !StartsWithASCII(url, resource_prefix_, /*case_sensitive=*/ true)) {
    return false;
  }

  return true;
}

// ContentSchemeRequestInterceptor
ContentSchemeRequestInterceptor::ContentSchemeRequestInterceptor() {
}

bool ContentSchemeRequestInterceptor::ShouldHandleRequest(
    const net::URLRequest* request) const {
  return request->url().SchemeIs(xwalk::kContentScheme);
}

// AppSchemeRequestInterceptor
AppSchemeRequestInterceptor::AppSchemeRequestInterceptor() {
}

bool AppSchemeRequestInterceptor::ShouldHandleRequest(
    const net::URLRequest* request) const {
  return request->url().SchemeIs(xwalk::kAppScheme);
}

}  // namespace

namespace xwalk {

bool RegisterAndroidProtocolHandler(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

// static
scoped_ptr<net::URLRequestInterceptor>
CreateContentSchemeRequestInterceptor() {
  return make_scoped_ptr<net::URLRequestInterceptor>(
      new ContentSchemeRequestInterceptor());
}

// static
scoped_ptr<net::URLRequestInterceptor> CreateAssetFileRequestInterceptor() {
  return scoped_ptr<net::URLRequestInterceptor>(
      new AssetFileRequestInterceptor());
}

// static
scoped_ptr<net::URLRequestInterceptor> CreateAppSchemeRequestInterceptor() {
  return make_scoped_ptr<net::URLRequestInterceptor>(
      new AppSchemeRequestInterceptor());
}


// Set a context object to be used for resolving resource queries. This can
// be used to override the default application context and redirect all
// resource queries to a specific context object, e.g., for the purposes of
// testing.
//
// |context| should be a android.content.Context instance or NULL to enable
// the use of the standard application context.
static void SetResourceContextForTesting(JNIEnv* env, jclass /*clazz*/,
                                         jobject context) {
  if (context) {
    ResetResourceContext(new JavaObjectWeakGlobalRef(env, context));
  } else {
    ResetResourceContext(NULL);
  }
}

static jstring GetAndroidAssetPath(JNIEnv* env, jclass /*clazz*/) {
  // OK to release, JNI binding.
  return ConvertUTF8ToJavaString(
      env, xwalk::kAndroidAssetPath).Release();
}

static jstring GetAndroidResourcePath(JNIEnv* env, jclass /*clazz*/) {
  // OK to release, JNI binding.
  return ConvertUTF8ToJavaString(
      env, xwalk::kAndroidResourcePath).Release();
}

}  // namespace xwalk
