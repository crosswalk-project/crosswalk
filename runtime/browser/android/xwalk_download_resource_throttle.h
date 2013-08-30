// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_DOWNLOAD_RESOURCE_THROTTLE_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_DOWNLOAD_RESOURCE_THROTTLE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/browser/resource_throttle.h"

namespace net {
class URLRequest;
}

namespace xwalk {

class XWalkDownloadResourceThrottle : public content::ResourceThrottle {
 public:
  XWalkDownloadResourceThrottle(net::URLRequest* request,
                                int render_process_id,
                                int render_view_id,
                                int request_id);
  virtual ~XWalkDownloadResourceThrottle();

  virtual void WillStartRequest(bool* defer) OVERRIDE;
  virtual void WillProcessResponse(bool* defer) OVERRIDE;

 private:
  const net::URLRequest* request_;
  int render_process_id_;
  int render_view_id_;
  int request_id_;

  DISALLOW_COPY_AND_ASSIGN(XWalkDownloadResourceThrottle);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_DOWNLOAD_RESOURCE_THROTTLE_H_
