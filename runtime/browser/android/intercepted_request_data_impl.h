// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_XWALK_INTERCEPTED_REQUEST_DATA_IMPL_H_
#define ANDROID_XWALK_INTERCEPTED_REQUEST_DATA_IMPL_H_

#include "intercepted_request_data.h"
#include "base/android/scoped_java_ref.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"

namespace xwalk {

class InputStream;

class InterceptedRequestDataImpl : public InterceptedRequestData {
 public:
  InterceptedRequestDataImpl(const base::android::JavaRef<jobject>& obj);
  virtual ~InterceptedRequestDataImpl();

  virtual scoped_ptr<InputStream> GetInputStream(JNIEnv* env) const;
  virtual bool GetMimeType(JNIEnv* env, std::string* mime_type) const;
  virtual bool GetCharset(JNIEnv* env, std::string* charset) const;

  virtual net::URLRequestJob* CreateJobFor(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) const OVERRIDE;

 private:
  base::android::ScopedJavaGlobalRef<jobject> java_object_;

  DISALLOW_COPY_AND_ASSIGN(InterceptedRequestDataImpl);
};

bool RegisterInterceptedRequestData(JNIEnv* env);

} // namespace xwalk

#endif  // ANDROID_XWALK_INTERCEPTED_REQUEST_DATA_IMPL_H_
