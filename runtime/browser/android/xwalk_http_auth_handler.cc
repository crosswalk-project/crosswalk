// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_http_auth_handler.h"

#include "xwalk/runtime/browser/android/xwalk_content.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"
#include "xwalk/runtime/browser/android/xwalk_login_delegate.h"
#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "content/public/browser/browser_thread.h"
#include "jni/XWalkHttpAuthHandler_jni.h"
#include "net/base/auth.h"
#include "content/public/browser/web_contents.h"

using base::android::ConvertJavaStringToUTF16;

namespace xwalk {

XWalkHttpAuthHandler::XWalkHttpAuthHandler(XWalkLoginDelegate* login_delegate,
                                           net::AuthChallengeInfo* auth_info,
                                           bool first_auth_attempt)
    : login_delegate_(login_delegate),
      host_(auth_info->challenger.host()),
      realm_(auth_info->realm) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  JNIEnv* env = base::android::AttachCurrentThread();
  http_auth_handler_.Reset(
      Java_XWalkHttpAuthHandler_create(
          env, reinterpret_cast<jint>(this), first_auth_attempt));
}

XWalkHttpAuthHandler:: ~XWalkHttpAuthHandler() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  Java_XWalkHttpAuthHandler_handlerDestroyed(
      base::android::AttachCurrentThread(),
      http_auth_handler_.obj());
}

void XWalkHttpAuthHandler::Proceed(JNIEnv* env,
                                   jobject obj,
                                   jstring user,
                                   jstring password) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  if (login_delegate_.get()) {
    login_delegate_->Proceed(ConvertJavaStringToUTF16(env, user),
                             ConvertJavaStringToUTF16(env, password));
    login_delegate_ = NULL;
  }
}

void XWalkHttpAuthHandler::Cancel(JNIEnv* env, jobject obj) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  if (login_delegate_.get()) {
    login_delegate_->Cancel();
    login_delegate_ = NULL;
  }
}

bool XWalkHttpAuthHandler::HandleOnUIThread(
    content::WebContents* web_contents) {
  DCHECK(web_contents);
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  XWalkContent* xwalk_content = XWalkContent::FromWebContents(web_contents);
  return xwalk_content->GetContentsClientBridge()->OnReceivedHttpAuthRequest(
      http_auth_handler_, host_, realm_);
}

// static
XWalkHttpAuthHandlerBase* XWalkHttpAuthHandlerBase::Create(
    XWalkLoginDelegate* login_delegate,
    net::AuthChallengeInfo* auth_info,
    bool first_auth_attempt) {
  return new XWalkHttpAuthHandler(login_delegate, auth_info,
                                  first_auth_attempt);
}

bool RegisterXWalkHttpAuthHandler(JNIEnv* env) {
  return RegisterNativesImpl(env) >= 0;
}

}  // namespace xwalk
