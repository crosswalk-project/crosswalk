// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_RESOURCE_RESPONSE_IMPL_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_RESOURCE_RESPONSE_IMPL_H_

#include <memory>
#include <string>
#include <vector>

#include "base/android/scoped_java_ref.h"
#include "base/compiler_specific.h"
#include "xwalk/runtime/browser/android/xwalk_web_resource_response.h"

namespace net {
class HttpResponseHeaders;
}

namespace xwalk {

class InputStream;

class XWalkWebResourceResponseImpl : public XWalkWebResourceResponse {
 public:
  // It is expected that |obj| is an instance of the Java-side
  // org.xwalk.core.XWalkWebResourceResponse class.
  explicit XWalkWebResourceResponseImpl(
        const base::android::JavaRef<jobject>& obj);
  ~XWalkWebResourceResponseImpl() override;

  std::unique_ptr<InputStream> GetInputStream(JNIEnv* env) const override;
  bool GetMimeType(JNIEnv* env, std::string* mime_type) const override;
  bool GetCharset(JNIEnv* env, std::string* charset) const override;
  bool GetPackageName(JNIEnv* env, std::string* name) const override;
  bool GetStatusInfo(JNIEnv* env,
                     int* status_code,
                     std::string* reason_phrase) const override;
  bool GetResponseHeaders(JNIEnv* env,
                          net::HttpResponseHeaders* headers) const override;

 private:
  base::android::ScopedJavaGlobalRef<jobject> java_object_;

  DISALLOW_COPY_AND_ASSIGN(XWalkWebResourceResponseImpl);
};

bool RegisterXWalkWebResourceResponse(JNIEnv* env);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_WEB_RESOURCE_RESPONSE_IMPL_H_
