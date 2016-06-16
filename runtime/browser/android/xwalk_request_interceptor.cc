// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_request_interceptor.h"

#include <memory>

#include "base/android/jni_string.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_request_info.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/url_request/url_request_job.h"
#include "xwalk/runtime/browser/android/xwalk_contents_io_thread_client.h"
#include "xwalk/runtime/browser/android/xwalk_web_resource_response.h"

using content::BrowserThread;
using content::RenderViewHost;
using content::ResourceRequestInfo;

namespace xwalk {

namespace {

const void* kURLRequestUserDataKey = &kURLRequestUserDataKey;

}  // namespace

XWalkRequestInterceptor::XWalkRequestInterceptor() {
}

XWalkRequestInterceptor::~XWalkRequestInterceptor() {
}

std::unique_ptr<XWalkWebResourceResponse>
XWalkRequestInterceptor::QueryForXWalkWebResourceResponse(
    const GURL& location,
    net::URLRequest* request) const {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  int render_process_id, render_frame_id;
  if (!ResourceRequestInfo::GetRenderFrameForRequest(
      request, &render_process_id, &render_frame_id))
    return std::unique_ptr<XWalkWebResourceResponse>();

  std::unique_ptr<XWalkContentsIoThreadClient> io_thread_client =
    XWalkContentsIoThreadClient::FromID(render_process_id, render_frame_id);

  if (!io_thread_client.get())
    return std::unique_ptr<XWalkWebResourceResponse>();

  return io_thread_client->ShouldInterceptRequest(location, request);
}

net::URLRequestJob* XWalkRequestInterceptor::MaybeInterceptRequest(
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate) const {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));

  // See if we've already found out the xwalk_web_resource_response for this
  // request.
  // This is done not only for efficiency reasons, but also for correctness
  // as it is possible for the Interceptor chain to be invoked more than once
  // (in which case we don't want to query the embedder multiple times).
  if (request->GetUserData(kURLRequestUserDataKey))
    return nullptr;

  request->SetUserData(kURLRequestUserDataKey,
                       new base::SupportsUserData::Data());

  std::unique_ptr<XWalkWebResourceResponse> xwalk_web_resource_response =
      QueryForXWalkWebResourceResponse(request->url(), request);

  if (!xwalk_web_resource_response)
    return nullptr;
  return XWalkWebResourceResponse::CreateJobFor(
    std::move(xwalk_web_resource_response), request, network_delegate);
}

}  // namespace xwalk
