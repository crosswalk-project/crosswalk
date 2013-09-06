// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_download_resource_throttle.h"

#include "content/public/browser/android/download_controller_android.h"
#include "content/public/browser/resource_controller.h"
#include "net/http/http_request_headers.h"
#include "net/url_request/url_request.h"

namespace xwalk {

XWalkDownloadResourceThrottle::XWalkDownloadResourceThrottle(
    net::URLRequest* request,
    int render_process_id,
    int render_view_id,
    int request_id)
    : request_(request),
      render_process_id_(render_process_id),
      render_view_id_(render_view_id),
      request_id_(request_id) {
}

XWalkDownloadResourceThrottle::~XWalkDownloadResourceThrottle() {
}

void XWalkDownloadResourceThrottle::WillStartRequest(bool* defer) {
}

void XWalkDownloadResourceThrottle::WillProcessResponse(bool* defer) {
  content::DownloadControllerAndroid::Get()->CreateGETDownload(
      render_process_id_, render_view_id_, request_id_);
  controller()->Cancel();
}

}  // namespace xwalk
