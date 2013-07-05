// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/runtime/browser/runtime_network_delegate.h"

#include "net/base/net_errors.h"
#include "net/base/static_cookie_policy.h"
#include "net/url_request/url_request.h"

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

int RuntimeNetworkDelegate::OnBeforeSendHeaders(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    net::HttpRequestHeaders* headers) {
  return net::OK;
}

void RuntimeNetworkDelegate::OnSendHeaders(
    net::URLRequest* request,
    const net::HttpRequestHeaders& headers) {
}

int RuntimeNetworkDelegate::OnHeadersReceived(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    const net::HttpResponseHeaders* original_response_headers,
    scoped_refptr<net::HttpResponseHeaders>* override_response_headers) {
  return net::OK;
}

void RuntimeNetworkDelegate::OnBeforeRedirect(net::URLRequest* request,
                                              const GURL& new_location) {
}

void RuntimeNetworkDelegate::OnResponseStarted(net::URLRequest* request) {
}

void RuntimeNetworkDelegate::OnRawBytesRead(const net::URLRequest& request,
                                            int bytes_read) {
}

void RuntimeNetworkDelegate::OnCompleted(net::URLRequest* request,
                                         bool started) {
}

void RuntimeNetworkDelegate::OnURLRequestDestroyed(net::URLRequest* request) {
}

void RuntimeNetworkDelegate::OnPACScriptError(int line_number,
                                              const string16& error) {
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
  // TODO(hmin): We need to define a policy for cookie read/write.
  return true;
}

bool RuntimeNetworkDelegate::OnCanSetCookie(const net::URLRequest& request,
                                            const std::string& cookie_line,
                                            net::CookieOptions* options) {
  // TODO(hmin): We need to define a policy for cookie read/write.
  return true;
}

bool RuntimeNetworkDelegate::OnCanAccessFile(const net::URLRequest& request,
                                             const base::FilePath& path) const {
  return true;
}

bool RuntimeNetworkDelegate::OnCanThrottleRequest(
    const net::URLRequest& request) const {
  return false;
}

int RuntimeNetworkDelegate::OnBeforeSocketStreamConnect(
    net::SocketStream* socket,
    const net::CompletionCallback& callback) {
  return net::OK;
}

void RuntimeNetworkDelegate::OnRequestWaitStateChange(
    const net::URLRequest& request,
    RequestWaitState waiting) {
}

}  // namespace xwalk
