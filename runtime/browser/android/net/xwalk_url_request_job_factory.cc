// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/net/xwalk_url_request_job_factory.h"

#include <string>

#include "net/base/net_errors.h"
#include "net/url_request/url_request_error_job.h"
#include "net/url_request/url_request_job_factory_impl.h"
#include "net/url_request/url_request_job_manager.h"

using net::NetworkDelegate;
using net::URLRequest;
using net::URLRequestJob;

namespace xwalk {

XWalkURLRequestJobFactory::XWalkURLRequestJobFactory()
    : next_factory_(new net::URLRequestJobFactoryImpl()) {
}

XWalkURLRequestJobFactory::~XWalkURLRequestJobFactory() {
}

bool XWalkURLRequestJobFactory::IsHandledProtocol(
    const std::string& scheme) const {
  // This introduces a dependency on the URLRequestJobManager
  // implementation. The assumption is that if true is returned from this
  // method it is still valid to return NULL from the
  // MaybeCreateJobWithProtocolHandler method and in that case the
  // URLRequestJobManager will try and create the URLRequestJob by using the
  // set of built in handlers.
  return true;
}

bool XWalkURLRequestJobFactory::IsHandledURL(const GURL& url) const {
  return true;
}

URLRequestJob* XWalkURLRequestJobFactory::MaybeCreateJobWithProtocolHandler(
    const std::string& scheme,
    URLRequest* request,
    NetworkDelegate* network_delegate) const {
  URLRequestJob* job = next_factory_->MaybeCreateJobWithProtocolHandler(
      scheme, request, network_delegate);

  if (job)
    return job;

  // If URLRequest supports the scheme NULL should be returned from this method.
  // In that case the built in handlers will then be used to create the job.
  // NOTE(joth): See the assumption in IsHandledProtocol above.
  if (net::URLRequest::IsHandledProtocol(scheme))
    return NULL;

  return new net::URLRequestErrorJob(
      request, network_delegate, net::ERR_UNKNOWN_URL_SCHEME);
}

bool XWalkURLRequestJobFactory::SetProtocolHandler(
    const std::string& scheme,
    ProtocolHandler* protocol_handler) {
  return next_factory_->SetProtocolHandler(scheme, protocol_handler);
}

}  // namespace xwalk
