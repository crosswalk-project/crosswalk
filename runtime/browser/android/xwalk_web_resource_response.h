// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_RESOURCE_RESPONSE_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_RESOURCE_RESPONSE_H_

#include <memory>
#include <string>

#include "base/android/jni_android.h"

namespace net {
class HttpResponseHeaders;
class NetworkDelegate;
class URLRequest;
class URLRequestJob;
}

namespace xwalk {

class InputStream;

// This class represents the Java-side data that is to be used to complete a
// particular URLRequest.
class XWalkWebResourceResponse {
 public:
  virtual ~XWalkWebResourceResponse() {}

  virtual std::unique_ptr<InputStream> GetInputStream(JNIEnv* env) const = 0;
  virtual bool GetMimeType(JNIEnv* env, std::string* mime_type) const = 0;
  virtual bool GetCharset(JNIEnv* env, std::string* charset) const = 0;
  virtual bool GetPackageName(JNIEnv* env, std::string* name) const = 0;
  virtual bool GetStatusInfo(JNIEnv* env,
                             int* status_code,
                             std::string* reason_phrase) const = 0;
  // If true is returned then |headers| contain the headers, if false is
  // returned |headers| were not updated.
  virtual bool GetResponseHeaders(
      JNIEnv* env,
      net::HttpResponseHeaders* headers) const = 0;

  // This creates a URLRequestJob for the |request| which will read data from
  // the |xwalk_web_resource_response| structure (instead of going to the
  // network or to the cache).
  // The newly created job takes ownership of |xwalk_web_resource_response|.
  static net::URLRequestJob* CreateJobFor(
      std::unique_ptr<XWalkWebResourceResponse> xwalk_web_resource_response,
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate);

 protected:
  XWalkWebResourceResponse() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkWebResourceResponse);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_RESOURCE_RESPONSE_H_
