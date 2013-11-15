// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_XWALK_HTTP_AUTH_HANDLER_H_
#define ANDROID_XWALK_HTTP_AUTH_HANDLER_H_

#include <jni.h>
#include <string>

#include "xwalk_http_auth_handler_base.h"
#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"
#include "base/memory/ref_counted.h"
#include "xwalk_login_delegate.h"

namespace content {
class WebContents;
};

namespace net {
class AuthChallengeInfo;
};

namespace xwalk {
class XWalkLoginDelegate;
// Native class for Java class of same name and owns an instance
// of that Java object.
// One instance of this class is created per underlying XWalkLoginDelegate.
class XWalkHttpAuthHandler : public XWalkHttpAuthHandlerBase {
 public:
  XWalkHttpAuthHandler(XWalkLoginDelegate* login_delegate,
                       net::AuthChallengeInfo* auth_info,
                       bool first_auth_attempt);
  virtual ~XWalkHttpAuthHandler();

  // from HttpAuthHandler
  virtual bool HandleOnUIThread(content::WebContents* web_contents) OVERRIDE;

  void Proceed(JNIEnv* env, jobject obj, jstring username, jstring password);
  void Cancel(JNIEnv* env, jobject obj);

 private:
  scoped_refptr<XWalkLoginDelegate> login_delegate_;
  base::android::ScopedJavaGlobalRef<jobject> http_auth_handler_;
  std::string host_;
  std::string realm_;
};

bool RegisterXWalkHttpAuthHandler(JNIEnv* env);

}  // namespace xwalk

#endif  // ANDROID_XWALK_HTTP_AUTH_HANDLER_H_
