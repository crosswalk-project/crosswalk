// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_network_delegate.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/resource_request_info.h"
#include "net/base/net_errors.h"
#include "net/base/static_cookie_policy.h"
#include "net/url_request/url_request.h"

#if defined(OS_ANDROID)
#include "xwalk/runtime/browser/android/xwalk_contents_io_thread_client.h"
#include "xwalk/runtime/browser/android/xwalk_cookie_access_policy.h"
#endif

using content::BrowserThread;

namespace xwalk {

RuntimeNetworkDelegate::RuntimeNetworkDelegate() {
}

RuntimeNetworkDelegate::~RuntimeNetworkDelegate() {
}

int RuntimeNetworkDelegate::OnBeforeURLRequest(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    GURL* new_url) {
  return net::OK;
}

int RuntimeNetworkDelegate::OnBeforeStartTransaction(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    net::HttpRequestHeaders* headers) {
  return net::OK;
}

void RuntimeNetworkDelegate::OnStartTransaction(
    net::URLRequest* request,
    const net::HttpRequestHeaders& headers) {
}

int RuntimeNetworkDelegate::OnHeadersReceived(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    const net::HttpResponseHeaders* original_response_headers,
    scoped_refptr<net::HttpResponseHeaders>* override_response_headers,
    GURL* allowed_unsafe_redirect_url) {
#if defined(OS_ANDROID)
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  int render_process_id, render_frame_id;
  if (content::ResourceRequestInfo::GetRenderFrameForRequest(
      request, &render_process_id, &render_frame_id)) {
    std::unique_ptr<XWalkContentsIoThreadClient> io_thread_client =
        XWalkContentsIoThreadClient::FromID(render_process_id, render_frame_id);
    if (io_thread_client.get()) {
      io_thread_client->OnReceivedResponseHeaders(request,
          original_response_headers);
    }
  }
#endif
  return net::OK;
}

void RuntimeNetworkDelegate::OnBeforeRedirect(net::URLRequest* request,
                                              const GURL& new_location) {
}

void RuntimeNetworkDelegate::OnResponseStarted(net::URLRequest* request) {
}

void RuntimeNetworkDelegate::OnNetworkBytesReceived(net::URLRequest* request,
                                                    int64_t bytes_received) {
}

void RuntimeNetworkDelegate::OnCompleted(net::URLRequest* request,
                                         bool started) {
}

void RuntimeNetworkDelegate::OnURLRequestDestroyed(net::URLRequest* request) {
}

void RuntimeNetworkDelegate::OnPACScriptError(int line_number,
                                              const base::string16& error) {
}

RuntimeNetworkDelegate::AuthRequiredResponse
RuntimeNetworkDelegate::OnAuthRequired(
    net::URLRequest* request,
    const net::AuthChallengeInfo& auth_info,
    const AuthCallback& callback,
    net::AuthCredentials* credentials) {
  return AUTH_REQUIRED_RESPONSE_NO_ACTION;
}

bool RuntimeNetworkDelegate::OnCanGetCookies(
    const net::URLRequest& request,
    const net::CookieList& cookie_list) {
#if defined(OS_ANDROID)
  return XWalkCookieAccessPolicy::GetInstance()->OnCanGetCookies(
    request, cookie_list);
#else
  return true;
#endif
}

bool RuntimeNetworkDelegate::OnCanSetCookie(const net::URLRequest& request,
                                            const std::string& cookie_line,
                                            net::CookieOptions* options) {
#if defined(OS_ANDROID)
  return XWalkCookieAccessPolicy::GetInstance()->OnCanSetCookie(request,
                                                                cookie_line,
                                                                options);
#else
  return true;
#endif
}

bool RuntimeNetworkDelegate::OnCanAccessFile(const net::URLRequest& request,
                                             const base::FilePath& path) const {
  return true;
}

}  // namespace xwalk
