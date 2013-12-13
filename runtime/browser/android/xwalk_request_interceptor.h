// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_REQUEST_INTERCEPTOR_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_REQUEST_INTERCEPTOR_H_

#include "base/memory/scoped_ptr.h"
#include "net/url_request/url_request_job_factory.h"

class GURL;

namespace net {
class URLRequest;
class URLRequestContextGetter;
class URLRequestJob;
class NetworkDelegate;
}

namespace xwalk {

class InterceptedRequestData;

// This class allows the Java-side embedder to substitute the default
// URLRequest of a given request for an alternative job that will read data
// from a Java stream.
class XWalkRequestInterceptor
    : public net::URLRequestJobFactory::ProtocolHandler {
 public:
  XWalkRequestInterceptor();
  virtual ~XWalkRequestInterceptor();

  // net::URLRequestJobFactory::ProtocolHandler override -----------------------
  virtual net::URLRequestJob* MaybeCreateJob(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) const OVERRIDE;

 private:
  scoped_ptr<InterceptedRequestData> QueryForInterceptedRequestData(
      const GURL& location,
      net::URLRequest* request) const;

  DISALLOW_COPY_AND_ASSIGN(XWalkRequestInterceptor);
};

}  //  namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_REQUEST_INTERCEPTOR_H_
