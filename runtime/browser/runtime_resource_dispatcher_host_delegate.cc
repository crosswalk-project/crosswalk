// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_resource_dispatcher_host_delegate.h"

#include "base/lazy_instance.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/resource_controller.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/resource_dispatcher_host_login_delegate.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/browser/resource_throttle.h"
#include "content/public/common/url_constants.h"
#include "net/base/load_flags.h"
#include "net/http/http_response_headers.h"
#include "net/url_request/url_request.h"

#if defined(OS_ANDROID)
#include "components/navigation_interception/intercept_navigation_delegate.h"
#include "xwalk/runtime/browser/android/xwalk_download_resource_throttle.h"
#endif

using content::BrowserThread;
using navigation_interception::InterceptNavigationDelegate;

namespace {
base::LazyInstance<xwalk::RuntimeResourceDispatcherHostDelegate>
    g_runtime_resource_dispatcher_host_delegate = LAZY_INSTANCE_INITIALIZER;
}

namespace xwalk {

RuntimeResourceDispatcherHostDelegate::RuntimeResourceDispatcherHostDelegate() {
}

RuntimeResourceDispatcherHostDelegate::
~RuntimeResourceDispatcherHostDelegate() {
}

// static
void RuntimeResourceDispatcherHostDelegate::ResourceDispatcherHostCreated() {
  content::ResourceDispatcherHost::Get()->SetDelegate(
      &g_runtime_resource_dispatcher_host_delegate.Get());
}

void RuntimeResourceDispatcherHostDelegate::RequestBeginning(
    net::URLRequest* request,
    content::ResourceContext* resource_context,
    appcache::AppCacheService* appcache_service,
    ResourceType::Type resource_type,
    int child_id,
    int route_id,
    ScopedVector<content::ResourceThrottle>* throttles) {
#if defined(OS_ANDROID)
  bool allow_intercepting =
      // We allow intercepting navigations within subframes, but only if the
      // scheme other than http or https. This is because the embedder
      // can't distinguish main frame and subframe callbacks (which could lead
      // to broken content if the embedder decides to not ignore the main frame
      // navigation, but ignores the subframe navigation).
      // The reason this is supported at all is that certain JavaScript-based
      // frameworks use iframe navigation as a form of communication with the
      // embedder.
      (resource_type == ResourceType::MAIN_FRAME ||
       (resource_type == ResourceType::SUB_FRAME &&
        !request->url().SchemeIs(content::kHttpScheme) &&
        !request->url().SchemeIs(content::kHttpsScheme)));
  if (allow_intercepting) {
    throttles->push_back(InterceptNavigationDelegate::CreateThrottleFor(
        request));
  }
#endif
}

void RuntimeResourceDispatcherHostDelegate::DownloadStarting(
    net::URLRequest* request,
    content::ResourceContext* resource_context,
    int child_id,
    int route_id,
    int request_id,
    bool is_content_initiated,
    bool must_download,
    ScopedVector<content::ResourceThrottle>* throttles) {
#if defined(OS_ANDROID)
  throttles->push_back(
      new XWalkDownloadResourceThrottle(
          request, child_id, route_id, request_id));
#endif
}

bool RuntimeResourceDispatcherHostDelegate::HandleExternalProtocol(
    const GURL& url,
    int child_id,
    int route_id) {
#if defined(OS_ANDROID)
  // On Android, there are many Uris need to be handled differently.
  // e.g: sms:, tel:, mailto: and etc.
  // So here return false to let embedders to decide which protocol
  // to be handled.
  return false;
#else
  return true;
#endif
}

}  // namespace xwalk
