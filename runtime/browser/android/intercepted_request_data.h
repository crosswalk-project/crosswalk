// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_INTERCEPTED_REQUEST_DATA_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_INTERCEPTED_REQUEST_DATA_H_

#include <string>

#include "base/memory/scoped_ptr.h"

namespace net {
class URLRequest;
class URLRequestJob;
class NetworkDelegate;
}

namespace xwalk {

// This class represents the Java-side data that is to be used to complete a
// particular URLRequest.
class InterceptedRequestData {
 public:
  virtual ~InterceptedRequestData() {}

  // This creates a URLRequestJob for the |request| wich will read data from
  // the |intercepted_request_data| structure (instead of going to the network
  // or to the cache).
  // The newly created job does not take ownership of |this|.
  virtual net::URLRequestJob* CreateJobFor(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) const = 0;

 protected:
  InterceptedRequestData() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(InterceptedRequestData);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_INTERCEPTED_REQUEST_DATA_H_
