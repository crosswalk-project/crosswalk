// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk_login_delegate.h"

#include "base/android/jni_android.h"
#include "base/logging.h"
#include "base/supports_user_data.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/browser/web_contents.h"
#include "net/base/auth.h"
#include "net/url_request/url_request.h"

using namespace base::android;

using content::BrowserThread;
using content::RenderViewHost;
using content::ResourceDispatcherHost;
using content::ResourceRequestInfo;
using content::WebContents;

namespace {
const char* kAuthAttemptsKey = "xwalk_auth_attempts";

class UrlRequestAuthAttemptsData : public base::SupportsUserData::Data {
 public:
  UrlRequestAuthAttemptsData() : auth_attempts_(0) { }
  int auth_attempts_;
};

}  // namespace

namespace xwalk {

XWalkLoginDelegate::XWalkLoginDelegate(net::AuthChallengeInfo* auth_info,
                                 net::URLRequest* request)
    : auth_info_(auth_info),
      request_(request),
      render_process_id_(0),
      render_view_id_(0) {
    ResourceRequestInfo::GetRenderViewForRequest(
        request, &render_process_id_, &render_view_id_);

    UrlRequestAuthAttemptsData* count =
        static_cast<UrlRequestAuthAttemptsData*>(
            request->GetUserData(kAuthAttemptsKey));

    if (count == NULL) {
      count = new UrlRequestAuthAttemptsData();
      request->SetUserData(kAuthAttemptsKey, count);
    }

    BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
        base::Bind(&XWalkLoginDelegate::HandleHttpAuthRequestOnUIThread,
                   this, (count->auth_attempts_ == 0)));
    count->auth_attempts_++;
}

XWalkLoginDelegate::~XWalkLoginDelegate() {
  // The Auth handler holds a ref count back on |this| object, so it should be
  // impossible to reach here while this object still owns an auth handler.
  DCHECK(xwalk_http_auth_handler_ == NULL);
}

void XWalkLoginDelegate::Proceed(const string16& user,
                                 const string16& password) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
      base::Bind(&XWalkLoginDelegate::ProceedOnIOThread,
                 this, user, password));
}

void XWalkLoginDelegate::Cancel() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
      base::Bind(&XWalkLoginDelegate::CancelOnIOThread, this));
}

void XWalkLoginDelegate::HandleHttpAuthRequestOnUIThread(
    bool first_auth_attempt) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  xwalk_http_auth_handler_.reset(XWalkHttpAuthHandlerBase::Create(
      this, auth_info_.get(), first_auth_attempt));

  RenderViewHost* render_view_host = RenderViewHost::FromID(
      render_process_id_, render_view_id_);
  if (!render_view_host) {
    Cancel();
    return;
  }

  WebContents* web_contents = WebContents::FromRenderViewHost(
      render_view_host);
  if (!xwalk_http_auth_handler_->HandleOnUIThread(web_contents)) {
    Cancel();
    return;
  }
}

void XWalkLoginDelegate::CancelOnIOThread() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  if (request_) {
    request_->CancelAuth();
    ResourceDispatcherHost::Get()->ClearLoginDelegateForRequest(request_);
    request_ = NULL;
  }
  DeleteAuthHandlerSoon();
}

void XWalkLoginDelegate::ProceedOnIOThread(const string16& user,
                                        const string16& password) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  if (request_) {
    request_->SetAuth(net::AuthCredentials(user, password));
    ResourceDispatcherHost::Get()->ClearLoginDelegateForRequest(request_);
    request_ = NULL;
  }
  DeleteAuthHandlerSoon();
}

void XWalkLoginDelegate::OnRequestCancelled() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  request_ = NULL;
  DeleteAuthHandlerSoon();
}

void XWalkLoginDelegate::DeleteAuthHandlerSoon() {
  if (!BrowserThread::CurrentlyOn(BrowserThread::UI)) {
    BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
        base::Bind(&XWalkLoginDelegate::DeleteAuthHandlerSoon, this));
    return;
  }
  xwalk_http_auth_handler_.reset();
}

}  // namespace xwalk
