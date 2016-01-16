// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_NET_XWALK_URL_REQUEST_JOB_FACTORY_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_NET_XWALK_URL_REQUEST_JOB_FACTORY_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "net/url_request/url_request_job_factory.h"

namespace net {
class URLRequestJobFactoryImpl;
}  // namespace net

namespace xwalk {

// Crosswalk uses a custom URLRequestJobFactoryImpl to support
// navigation interception and URLRequestJob interception for navigations to
// url with unsupported schemes.
// This is achieved by returning a URLRequestErrorJob for schemes that would
// otherwise be unhandled, which gives the embedder an opportunity to intercept
// the request.
class XWalkURLRequestJobFactory : public net::URLRequestJobFactory {
 public:
  XWalkURLRequestJobFactory();
  ~XWalkURLRequestJobFactory() override;

  bool SetProtocolHandler(const std::string& scheme,
                          scoped_ptr<ProtocolHandler> protocol_handler);

  // net::URLRequestJobFactory implementation.
  virtual net::URLRequestJob* MaybeCreateJobWithProtocolHandler(
      const std::string& scheme,
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) const override;

  net::URLRequestJob* MaybeInterceptRedirect(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate,
      const GURL& location) const override;

  net::URLRequestJob* MaybeInterceptResponse(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) const override;

  bool IsHandledProtocol(const std::string& scheme) const override;
  bool IsHandledURL(const GURL& url) const override;
  bool IsSafeRedirectTarget(const GURL& location) const override;

 private:
  // By default calls are forwarded to this factory, to avoid having to
  // subclass an existing implementation class.
  scoped_ptr<net::URLRequestJobFactoryImpl> next_factory_;

  DISALLOW_COPY_AND_ASSIGN(XWalkURLRequestJobFactory);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_NET_XWALK_URL_REQUEST_JOB_FACTORY_H_
