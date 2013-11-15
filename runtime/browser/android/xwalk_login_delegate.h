// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_LOGIN_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_LOGIN_DELEGATE_H_

#include "xwalk/runtime/browser/android/xwalk_http_auth_handler_base.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string16.h"
#include "content/public/browser/resource_dispatcher_host_login_delegate.h"

namespace net {
class AuthChallengeInfo;
class URLRequest;
}

namespace xwalk {

class XWalkLoginDelegate
    : public content::ResourceDispatcherHostLoginDelegate {
 public:
  XWalkLoginDelegate(net::AuthChallengeInfo* auth_info,
                     net::URLRequest* request);

  virtual void Proceed(const string16& user, const string16& password);
  virtual void Cancel();

  // from ResourceDispatcherHostLoginDelegate
  virtual void OnRequestCancelled() OVERRIDE;

 private:
  virtual ~XWalkLoginDelegate();
  void HandleHttpAuthRequestOnUIThread(bool first_auth_attempt);
  void CancelOnIOThread();
  void ProceedOnIOThread(const string16& user, const string16& password);
  void DeleteAuthHandlerSoon();

  scoped_ptr<XWalkHttpAuthHandlerBase> xwalk_http_auth_handler_;
  scoped_refptr<net::AuthChallengeInfo> auth_info_;
  net::URLRequest* request_;
  int render_process_id_;
  int render_view_id_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_LOGIN_DELEGATE_H_
