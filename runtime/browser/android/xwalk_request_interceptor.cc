// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_request_interceptor.h"

#include "base/android/jni_string.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_request_info.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/url_request/url_request_job.h"
#include "xwalk/runtime/browser/android/intercepted_request_data.h"
#include "xwalk/runtime/browser/android/xwalk_contents_io_thread_client.h"

using content::BrowserThread;
using content::RenderViewHost;
using content::ResourceRequestInfo;

namespace xwalk {

namespace {

const void* kURLRequestUserDataKey = &kURLRequestUserDataKey;

class URLRequestUserData : public base::SupportsUserData::Data {
 public:
    URLRequestUserData(
        scoped_ptr<InterceptedRequestData> intercepted_request_data)
        : intercepted_request_data_(intercepted_request_data.Pass()) {
    }

    static URLRequestUserData* Get(net::URLRequest* request) {
      return reinterpret_cast<URLRequestUserData*>(
          request->GetUserData(kURLRequestUserDataKey));
    }

    const InterceptedRequestData* intercepted_request_data() const {
      return intercepted_request_data_.get();
    }

 private:
  scoped_ptr<InterceptedRequestData> intercepted_request_data_;
};

}  // namespace

XWalkRequestInterceptor::XWalkRequestInterceptor() {
}

XWalkRequestInterceptor::~XWalkRequestInterceptor() {
}

scoped_ptr<InterceptedRequestData>
XWalkRequestInterceptor::QueryForInterceptedRequestData(
    const GURL& location,
    net::URLRequest* request) const {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  int render_process_id, render_view_id;
  if (!ResourceRequestInfo::GetRenderFrameForRequest(
      request, &render_process_id, &render_view_id))
    return scoped_ptr<InterceptedRequestData>();

  scoped_ptr<XWalkContentsIoThreadClient> io_thread_client =
    XWalkContentsIoThreadClient::FromID(render_process_id, render_view_id);

  if (!io_thread_client.get())
    return scoped_ptr<InterceptedRequestData>();

  return io_thread_client->ShouldInterceptRequest(location, request).Pass();
}

net::URLRequestJob* XWalkRequestInterceptor::MaybeCreateJob(
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate) const {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));

  // See if we've already found out the intercepted_request_data for this
  // request.
  // This is done not only for efficiency reasons, but also for correctness
  // as it is possible for the Interceptor chain to be invoked more than once
  // (in which case we don't want to query the embedder multiple times).
  URLRequestUserData* user_data = URLRequestUserData::Get(request);

  if (!user_data) {
    // To ensure we only query the embedder once, we rely on the fact that the
    // user_data object will be created and attached to the URLRequest after a
    // call to QueryForInterceptedRequestData is made (regardless of whether
    // the result of that call is a valid InterceptedRequestData* pointer or
    // NULL.
    user_data = new URLRequestUserData(
        QueryForInterceptedRequestData(request->url(), request));
    request->SetUserData(kURLRequestUserDataKey, user_data);
  }

  const InterceptedRequestData* intercepted_request_data =
      user_data->intercepted_request_data();

  if (!intercepted_request_data)
    return NULL;
  return intercepted_request_data->CreateJobFor(request, network_delegate);
}

}  // namespace xwalk
